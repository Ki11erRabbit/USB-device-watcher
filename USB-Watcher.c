#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct CmdSet {
	char *idVendor;
	char *idProduct;
	char *onPlugIn;
	char *onUnplug;
	char activated;
	char found;
} CmdSet;


void cat (char* buffer, char* initialDirectory, char* newDirectory, char key) {

    for (int i = 0; i < 50; i++) {
        buffer[i] = '\0';
    }


    strcat(buffer, initialDirectory);

    strcat(buffer, newDirectory);
    if (key == 0) {
        strcat(buffer, "/");
    }

}

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
    struct dirent *de; // Pointer for directory entry
    struct dirent *nde; // Pointer for directory entry
    // opendir() returns a pointer of DIR type.
    
    char * homeDir = getenv("HOME");
    //printf("%s\n", homeDir);
    char initialDirectory[21] = "/sys/bus/usb/devices/";
    char buffer[50];
    char found = 0;
    char activated = 0;

    CmdSet* devices[50];
    size_t totalDevices = 0;

    cat(buffer,homeDir, "/.config",0);
    DIR *homeDr = opendir(buffer);

    if (homeDr == NULL) {
	    printf("Could not open user home's .config\n");
	    return 1;
    }

    while ((de = readdir(homeDr)) != NULL) {
	    if (strstr(de->d_name, "USB-watcher.conf")) {
		    char tempBuffer[50];
		    cat (tempBuffer, buffer, "USB-watcher.conf",1);
		    //printf ("%s\n", tempBuffer);

		    FILE *fptr = fopen(tempBuffer, "r");
		    if (fptr == NULL) {
			    printf("Unable to open file\n");
			    return 1;
		    }
		    
		    //TODO: put in logic for reading lines
		    char *lineBuffer = NULL;
		    size_t lineBufferSize = 50;
		    char lineCount = 0;
		    char *vendorId = NULL, *productId = NULL,*plugIn = NULL,*unplug = NULL;
		    
		    while (getline(&lineBuffer,&lineBufferSize,fptr)) {
			    if (lineBuffer[0] == '\0') break;
			    if (lineBuffer[0] == '\n') goto skip;

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
		    fclose(fptr);
	    }
    }
    closedir(homeDr);
	

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while (1) {
        DIR *dr = opendir(initialDirectory);

        if (dr == NULL) // opendir returns NULL if couldn't open directory
        {
            printf("Could not open current directory" );
            continue;
        }

        while ((de = readdir(dr)) != NULL) {

            if (strstr(de->d_name, "-")) {
                cat(buffer, initialDirectory, de->d_name, 0);
                DIR *ndr = opendir(buffer);
                if (ndr == NULL) {
                    printf("Could not open current directory");
                    break;
                }
                char idVendor[8];
                char idProduct[8];
                while ((nde = readdir(ndr)) != NULL) {

                    if (strstr(nde->d_name, "idVendor")) {
                        //printf("\t%s\n", buffer);
                        char newbuff[50];
                        cat(newbuff, buffer, nde->d_name, 1);
                        //printf("\t\t%s\n", newbuff);

                        FILE *fptr = fopen(newbuff, "r");

                        if (fptr == NULL) {
                            printf("\tUnable to open file\n");
                            break;
                        }

                        fgets(idVendor, 8, fptr);
                        //printf("\t\t%s\n", fileContents);
                        fclose(fptr);

                    }
                    if (strstr(nde->d_name, "idProduct")) {
                        char newbuff[50];
                        cat(newbuff, buffer, nde->d_name, 1);
                        //printf("\t\t%s\n", newbuff);

                        FILE *fptr = fopen(newbuff, "r");

                        if (fptr == NULL) {
                            printf("\tUnable to open file\n");
                            break;
                        }

                        fgets(idProduct, 8, fptr);
                        //printf("\t\t%s\n", fileContents);
                        fclose(fptr);
                    }

                }
		for (size_t i = 0; i < totalDevices; i++) {
					
                	if (strstr(idVendor, devices[i]->idVendor) && strstr(idProduct, devices[i]->idProduct)) {
                    		//printf("\t%s/%s\n", idVendor, idProduct);
                    		devices[i]->found = 1;
                    		for (int i = 0; i < 8; i++) {
                        		idVendor[i] = '\0';
                        		idProduct[i] = '\0';
                    		}
                	}


		}


                closedir(ndr);
                //printf("%s\n", de->d_name);
            }
        }
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
				sleep(1);
		}
		else {
			if (devices[i]->activated == 1) {
				printf("USB device detached\n");
				system(devices[i]->onUnplug);
				devices[i]->activated = 0;
			}
			else {
				printf("USB device unplugged\n");
			}
			sleep(1);
		}
		devices[i]->found = 0;

	}
       /* if (found == 1) {
            printf("USB device attached\n");
            if (model_m->activated == 0) {
                sleep(1);
                system(model_m->onPlugIn);
                model_m->activated = 1;

                sleep(5);
            } else {
                sleep(5);
            }
        } else {
            printf("USB device detached\n");
            sleep(1);
            system(model_m->onUnplug);
            model_m->activated = 0;
        }
        found = 0;*/
        closedir(dr);
    }
    return 0;
}
