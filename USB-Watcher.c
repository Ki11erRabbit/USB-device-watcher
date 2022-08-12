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

	return temp;
}



int main(void)
{
    struct dirent *de; // Pointer for directory entry
    struct dirent *nde; // Pointer for directory entry
    // opendir() returns a pointer of DIR type.

    char initialDirectory[21] = "/sys/bus/usb/devices/";
    char buffer[50];
    char found = 0;
    char activated = 0;

    CmdSet* model_m;
    model_m = setup("17f6", "0822", "model-m-helper", "setxkbmap -layout us"); 

    // Refer http://pubs.opengroup.org/onlinepubs/7990989775/xsh/readdir.html
    // for readdir()
    while (1) {
        DIR *dr = opendir(initialDirectory);

        if (dr == NULL) // opendir returns NULL if couldn't open directory
        {
            printf("Could not open current directory" );
            return 0;
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
                if (strstr(idVendor, model_m->idVendor) && strstr(idProduct, model_m->idProduct)) {
                    //printf("\t%s/%s\n", idVendor, idProduct);
                    found = 1;
                    for (int i = 0; i < 8; i++) {
                        idVendor[i] = '\0';
                        idProduct[i] = '\0';
                    }
                    break;
                }

                closedir(ndr);
                //printf("%s\n", de->d_name);
            }
        }
        if (found == 1) {
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
        found = 0;
        closedir(dr);
    }
    return 0;
}
