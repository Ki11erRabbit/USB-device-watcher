#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXDEVICES 50

typedef struct CmdSet {
	char *idVendor;
	char *idProduct;
	char *onPlugIn;
	char *onUnplug;
	char activated;
	char found;
} CmdSet;


/*
 * cat takes in a buffer and two strings and a key value
 * the buffer is cleared and NULLed out
 * then the two strings are concatinated with the buffer
 * the key value dictates weather or not the buffer needs a trailing 
 * forward slash to reprsent a directory and not a file
 */

void cat(char* buffer, char* initialDirectory, char* newDirectory, char key) {

    for (int i = 0; i < 50; i++) {
        buffer[i] = '\0';
    }


    strcat(buffer, initialDirectory);

    strcat(buffer, newDirectory);
    if (key == 0) {
        strcat(buffer, "/");
    }

}

/*
 * returns a pointer to a struct that has the vendor and product ids
 * stored as well as the the commands to be run at plug in and unplug
 * and two flags to indicate if the commands have been run or not and 
 * if the device has been found in the list of devices
 */
CmdSet* setup(char* vendorId,char* productId, char* onPlugIn, char* onUnplug) {
	CmdSet *temp = malloc(sizeof(CmdSet));
	temp->idVendor = strdup(vendorId);
	temp->idProduct = strdup(productId);
	temp->onPlugIn = strdup(onPlugIn);
	temp->onUnplug = strdup(onUnplug);
	temp->activated = 0;
	temp->found = 0;

	return temp;
}



int main(void)
{
    // opendir() returns a pointer of DIR type.
    
    //printf("%s\n", homeDir);
    char buffer[50];

CmdSet* devices[MAXDEVICES];
    size_t totalDevices = 0;

	/*
	 *	Reads from /home/<user's home>/.config/USB-watcher.conf
	 *	format is vendorID/ProductID
	 *	the command you wish to execute on plugin
	 *	the command you wish to execute on unplug
	 *
	 *	when reading it ignores newlines so you can format it however you wish
	 *	comments begin with the '#' character
	 *	cannot go over more than MAXDEVICES usb devices which by default is 50
	 *
	 */
    
    char *homeDir = getenv("HOME");
    cat(buffer,homeDir, "/.config",0);
    DIR *homeDr = opendir(buffer);

    if (homeDr == NULL) {
	    printf("Could not open user home's .config\n");
	    return 1;
    }
    struct dirent *configEntry;
    while ((configEntry = readdir(homeDr)) != NULL) {// returns a directory entry pointer to files in /home/<user's home>/.config
	    if (strstr(configEntry->d_name, "USB-watcher.conf")) {
		    char pathToConfig[50];
		    cat (pathToConfig, buffer, "USB-watcher.conf",1);
		    //printf ("%s\n", tempBuffer);

		    FILE *configFile = fopen(pathToConfig, "r");
		    if (configFile == NULL) {
			    printf("Unable to open file\n");
			    return 1;
		    }
		    
		    char *lineBuffer = NULL;
		    size_t lineBufferSize = 50;
		    char lineCount = 0;
		    char *vendorId = NULL, *productId = NULL,*plugIn = NULL,*unplug = NULL;
		    
		    while (getline(&lineBuffer,&lineBufferSize,configFile)) {
			    if (lineBuffer[0] == '\0') break;
			    if (lineBuffer[0] == '\n' || lineBuffer[0] == '#') goto skip;

			    //printf("%s", lineBuffer);
			    lineCount++;
			    if (lineCount -1 == 0 ) {
				vendorId = strtok(lineBuffer,"/");
			    	productId = strtok(NULL,"\n");
				//printf("%s/%s\n", vendorId, productId);
			    }
			    else if (lineCount -1 == 1) plugIn = strtok(lineBuffer,"\n");
			    else if (lineCount -1 == 2) {
				unplug = strtok(lineBuffer,"\n");
				lineCount = 0;
				if (totalDevices == MAXDEVICES) {
					printf("Error!! Too many entries\n");
					break;
				}
				devices[totalDevices] = setup(vendorId, productId, plugIn,unplug);
				totalDevices++;
				vendorId = NULL;
				productId = NULL;
				plugIn = NULL;
				unplug = NULL;
			    }

			    skip:
			    lineBuffer = NULL;
		    }
		    fclose(configFile);
	    }
    }
    closedir(homeDr);
	

    /*
     * After setup, the code enters into an infinite loop
     * to become a daemon to watch usb devices by looping
     * through /sys/bus/usb/devices/ entries and checking 
     * if those entries have an idVendor and an idProduct
     * file in them. The code will then open those files
     * and check them against all of the devices to find
     * a match to see if it is plugged in. The code executes
     * every second.
     *
     */


    struct dirent *de; // Pointer for directory entry
    struct dirent *nde; // Pointer for directory entry
    char initialDirectory[21] = "/sys/bus/usb/devices/";
	
    while (1) {//infinite loop to become a daemon
        DIR *dr = opendir(initialDirectory);

        if (dr == NULL) // opendir returns NULL if couldn't open directory
        {
            printf("Could not open current directory" );
            continue;
        }

        while ((de = readdir(dr)) != NULL) {//loops through all entries in the directory '/sys/bus/usb/devices/'

            if (strstr(de->d_name, "-")) {
                cat(buffer, initialDirectory, de->d_name, 0);
                DIR *ndr = opendir(buffer);
                if (ndr == NULL) {
                    printf("Could not open current directory");
                    break;
                }
                char idVendor[8];
                char idProduct[8];
                while ((nde = readdir(ndr)) != NULL) {//loops though all files in '/sys/bus/usb/devices/*/'

                    if (strstr(nde->d_name, "idVendor")) {
                        //printf("\t%s\n", buffer);
                        char newbuff[50];
                        cat(newbuff, buffer, nde->d_name, 1);
                        //printf("\t\t%s\n", newbuff);

                        FILE *vendorFile = fopen(newbuff, "r");

                        if (vendorFile == NULL) {
                            printf("\tUnable to open file\n");
                            break;
                        }

                        fgets(idVendor, 8, vendorFile);
                        //printf("\t\t%s\n", fileContents);
                        fclose(vendorFile);

                    }
                    if (strstr(nde->d_name, "idProduct")) {
                        char newbuff[50];
                        cat(newbuff, buffer, nde->d_name, 1);
                        //printf("\t\t%s\n", newbuff);

                        FILE *productFile = fopen(newbuff, "r");

                        if (productFile == NULL) {
                            printf("\tUnable to open file\n");
                            break;
                        }

                        fgets(idProduct, 8, productFile);
                        //printf("\t\t%s\n", fileContents);
                        fclose(productFile);
                    }

                }
		for (size_t i = 0; i < totalDevices; i++) {//checks to see if the current ids match any of the products
			if (devices[i]->found == 1) continue;//to skip over devices that have already been found.

                	if (strstr(idVendor, devices[i]->idVendor) && strstr(idProduct, devices[i]->idProduct)) {
                    		//printf("\t%s/%s\n", idVendor, idProduct);
                    		devices[i]->found = 1;
                    		for (int i = 0; i < 8; i++) {
                        		idVendor[i] = '\0';
                        		idProduct[i] = '\0';
                    		}
				break;// skips looping through remaining devices. If you want multple commands to be executed use a script!
                	}


		}


                closedir(ndr);
                //printf("%s\n", de->d_name);
            }
        }
	/*
	 * loops through all devices and runs the associated
	 * commands if they have been found. Also runs the 
	 * associated commands for when the device is also
	 * unplugged from the machine.
	 */
	for (size_t i = 0; i < totalDevices; i++) {
		if (devices[i]->found == 1) {
			if (devices[i]->activated == 0) {
				printf("USB devices attached\n");
				system(devices[i]->onPlugIn);
				devices[i]->activated = 1;
			}
			else {
				printf("USB device still plugged in\n");
			}
		}
		else {// checking for if it has been activated is the perfect test to not constantly run the unplug command
			if (devices[i]->activated == 1) { 
				printf("USB device detached\n");
				system(devices[i]->onUnplug);
				devices[i]->activated = 0;
			}
			else {
				printf("USB device unplugged\n");
			}
		}
		devices[i]->found = 0;

	}
        closedir(dr);
	sleep(1);
    }
    return 0;
}
