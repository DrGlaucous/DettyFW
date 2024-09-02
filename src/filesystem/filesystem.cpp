#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
#include <time.h>
//#include <optional>


/* You only need to format LittleFS the first time you run a
   test or else use the LITTLEFS plugin to create a partition
   https://github.com/lorol/arduino-esp32littlefs-plugin */
   
#define FORMAT_LITTLEFS_IF_FAILED true

namespace filesystem {

    bool listDir(const char * dirname, uint8_t levels){
        //Serial.printf("Listing directory: %s\r\n", dirname);

        //fs::FS &fs = LittleFS;

        File root = LittleFS.open(dirname);
        if(!root){
            //Serial.println("- failed to open directory");
            return false;
        }
        if(!root.isDirectory()){
            //Serial.println(" - not a directory");
            return false;
        }

        File file = root.openNextFile();
        while(file){
            if(file.isDirectory()){
                Serial.print("  DIR : ");

                Serial.print(file.name());
                time_t t= file.getLastWrite();
                struct tm * tmstruct = localtime(&t);
                Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);

                if(levels){
                    listDir(file.name(), levels -1);
                }
            } else {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("  SIZE: ");

                Serial.print(file.size());
                time_t t= file.getLastWrite();
                struct tm * tmstruct = localtime(&t);
                Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
            }
            file = root.openNextFile();
        }
        return true;
    }

    bool createDir(const char * path){
        //Serial.printf("Creating Dir: %s\n", path);
        if(!LittleFS.mkdir(path)){
            return false;
        }

        return true;
    }

    bool removeDir(const char * path){
        //Serial.printf("Removing Dir: %s\n", path);
        if(!LittleFS.rmdir(path)){
            return false;
        }
        return true;
    }

    bool readFile(const char * path, uint8_t** buffer, size_t* size){
        //Serial.printf("Reading file: %s\r\n", path);

        File file = LittleFS.open(path);
        if(!file || file.isDirectory()){
            //Serial.println("- failed to open file for reading");
            return false;
        }

        uint8_t* filebuf = (uint8_t*)malloc(file.size());
        size_t read_size = file.readBytes((char*)filebuf, file.size());

        if(read_size != file.size()) {
            free(filebuf);
            file.close();
            return false;
        }

        //successful read, pass out pointer to filebuffer
        *buffer = filebuf;

        file.close();

        return true;

    }

    //read to serial terminal (for debug)
    void readFile2(const char * path){
        Serial.printf("Reading file: %s\r\n", path);

        File file = LittleFS.open(path);
        if(!file || file.isDirectory()){
            Serial.println("- failed to open file for reading");
            return;
        }

        Serial.println("- read from file:");
        while(file.available()){
            Serial.write(file.read());
        }
        file.close();
    }

    bool writeFile(const char * path, const uint8_t * buffer, size_t size){
        //Serial.printf("Writing file: %s\r\n", path);

        File file = LittleFS.open(path, FILE_WRITE);
        if(!file){
            return false;
        }

        size_t write_size = file.write(buffer, size);

        file.close();

        if(write_size != size){
            return false;
        }

        return true;
    }

    bool appendFile(const char * path, const uint8_t * buffer, size_t size){
        //Serial.printf("Appending to file: %s\r\n", path);

        File file = LittleFS.open(path, FILE_APPEND);
        if(!file){
            return false;
        }

        size_t write_size = file.write(buffer, size);

        file.close();

        if(write_size != size){
            return false;
        }

        return true;
    }

    bool renameFile(const char * path1, const char * path2){
        //Serial.printf("Renaming file %s to %s\r\n", path1, path2);
        if (!LittleFS.rename(path1, path2)) {
            return false;
        }

        return true;
    }

    bool deleteFile(const char * path){
        //Serial.printf("Deleting file: %s\r\n", path);
        if(!LittleFS.remove(path)){
            return false;
        }

        return true;
    }

    bool initialize() {
        if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
            return false;
        }
        return true;
    };

}

/*
// SPIFFS-like write and delete file

// See: https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.cpp#L60
void writeFile2(fs::FS &fs, const char * path, const char * message){
    if(!fs.exists(path)){
		if (strchr(path, '/')) {
            Serial.printf("Create missing folders of: %s\r\n", path);
			char *pathStr = strdup(path);
			if (pathStr) {
				char *ptr = strchr(pathStr, '/');
				while (ptr) {
					*ptr = 0;
					fs.mkdir(pathStr);
					*ptr = '/';
					ptr = strchr(ptr+1, '/');
				}
			}
			free(pathStr);
		}
    }

    Serial.printf("Writing file to: %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("- file written");
    } else {
        Serial.println("- write failed");
    }
    file.close();
}

// See:  https://github.com/esp8266/Arduino/blob/master/libraries/LittleFS/src/LittleFS.h#L149
void deleteFile2(fs::FS &fs, const char * path){
    Serial.printf("Deleting file and empty folders on path: %s\r\n", path);

    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }

    char *pathStr = strdup(path);
    if (pathStr) {
        char *ptr = strrchr(pathStr, '/');
        if (ptr) {
            Serial.printf("Removing all empty folders on path: %s\r\n", path);
        }
        while (ptr) {
            *ptr = 0;
            fs.rmdir(pathStr);
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
}
*/


void example(){
    Serial.begin(115200);
    
    filesystem::initialize();

    filesystem::listDir("/", 1);
    
    uint8_t* buffer;
    size_t size;
    filesystem::readFile("/test.txt", &buffer, &size);
    Serial.print((char*)buffer);
    free(buffer);

    Serial.printf( "\nTest complete\n" );
}
