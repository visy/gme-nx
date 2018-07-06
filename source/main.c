#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <malloc.h>
#include <math.h>

#include <switch.h>
#include <limits.h>

#include "gme.h"

#define SAMPLERATE 48000
#define CHANNELCOUNT 2
#define FRAMERATE (1000 / 30)
#define BYTESPERSAMPLE 2

gme_t* emu;
gme_err_t error;

DIR* dir;
struct dirent* ent;

AudioOutBuffer audout_buf;
AudioOutBuffer *released_out_buffer;

// Make sure the sample buffer size is aligned to 0x1000 bytes.
u32 data_size;
u32 buffer_size;

// Allocate the buffer.
short int* out_buf_data;

Result rc = 0;

gme_err_t gme_load_custom( gme_t*, gme_reader_t, long file_size, void* your_data );



int cursor_y = 0;
char current_name[256] = "";

void print_dir() {
    consoleClear();
    int ii = 0;
    dir = opendir(""); //Open current-working-directory.
    if(dir==NULL)
    {
        printf("Failed to open dir.\n");
    }
    else
    {
        while ((ent = readdir(dir)))
        {
            if (ii == cursor_y) { 
                printf("> %s\n", ent->d_name); 
                strcpy(current_name,ent->d_name);
            }
            else printf("  %s\n", ent->d_name);
            ii++;
        }
        closedir(dir);
    }

    if (ii == 0) printf("No files found in current directory.\n");
}

int playing = 0;
unsigned char buf2[100000000] = {0};

int offsett = 0;

gme_err_t ReaderBlock (void *data, void *buf, long length)
{
    memcpy(buf,buf2+offsett, length);

    offsett+=length;

    return NULL;
}

void playtrack() {
    if (R_SUCCEEDED(rc))
    {

        consoleClear();

        if (playing == 1) rc = audoutStopAudioOut();
    
        printf("Sample rate: 0x%x\n", audoutGetSampleRate());
        printf("Channel count: 0x%x\n", audoutGetChannelCount());
        printf("PCM format: 0x%x\n", audoutGetPcmFormat());
        printf("Device state: 0x%x\n", audoutGetDeviceState());

        FILE* file;
        file = fopen(current_name, "rb");

        unsigned char buf[4] = {0};
        size_t bytes = 0, i, readsz = sizeof buf;

        bytes = fread(buf, 4, 1, file);

        fclose(file);

        file = fopen(current_name, "rb");

        fseek(file, 0, SEEK_END); // seek to end of file
        size_t filesize = ftell(file); // get current file pointer
        fseek(file, 0, SEEK_SET); // seek back to beginning of file

        size_t bytes2 = 0;

        fclose(file);

        printf("file length: %ld\n", filesize);

        printf("header: '");
        printf("%c",buf[0]);
        printf("%c",buf[1]);
        printf("%c",buf[2]);
        printf("%c",buf[3]);
        printf("'\n");

        const char *type = gme_identify_header(&buf);

        if (!*type) {
            printf("error: unknown file type\n");
            return;
        }

        printf("*** Detected file type %s\n", type);

        emu = gme_new_emu (gme_identify_extension(type), 48000);

        if (emu == NULL)
        {
            printf("error initialing gme\n");
            return;
        }

        file = fopen(current_name, "rb");

        bytes2 = fread(buf2, sizeof *buf2, filesize, file);

        gme_load_custom (emu, ReaderBlock, filesize, &buf2);

        // Start audio playback.
        rc = audoutStartAudioOut();
        printf("audoutStartAudioOut() returned 0x%x\n", rc);

        gme_start_track( emu, 0 );
        playing = 1;

    }

}

int ccc = 0;
 u32 released_out_count;

int main(int argc, char **argv)
{
    
    gfxInitDefault();

    // Initialize console. Using NULL as the second argument tells the console library to use the internal console structure as current one.
    consoleInit(NULL);

    // Allocate the buffer.
    out_buf_data = memalign(0x1000, buffer_size);
    
    // Ensure buffers were properly allocated.
    if (out_buf_data == NULL)
    {
        rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
        printf("Failed to allocate sample data buffers\n");
    }
    
    if (R_SUCCEEDED(rc))
        memset(out_buf_data, 0, buffer_size);
    
    if (R_SUCCEEDED(rc))
    {
        // Initialize the default audio output device.
        rc = audoutInitialize();
        printf("audoutInitialize() returned 0x%x\n", rc);
    }
    
    print_dir();

    released_out_buffer = NULL;
    released_out_count = 0;

    while (appletMainLoop())
    {
        //Scan all the inputs. This should be done once for each frame
        hidScanInput();

        //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS) break; // break in order to return to hbmenu
        
        if (kDown & KEY_A)
        {
            playtrack();
        }
        
        if (kDown & KEY_B)
        {
            printf("Press b\n");
        }
        
        if (kDown & KEY_DLEFT)
        {
            printf("Press left\n");
        }
        
        if (kDown & KEY_DUP)
        {
            if (cursor_y > 0) cursor_y--;
            print_dir();
        }
        
        if (kDown & KEY_DRIGHT)
        {
            printf("Press right\n");
        }
        
        if (kDown & KEY_DDOWN)
        {
            if (cursor_y < 40) cursor_y++;
            print_dir();
        }
        
        if (kDown & KEY_L)
        {
        }
        
        if (kDown & KEY_R)
        {
        }
        
        if (kDown & KEY_ZL)
        {
        }
        
        if (kDown & KEY_ZR)
        {
        }
        
        if (R_SUCCEEDED(rc) && playing == 1)
        {
                long int SAMPLECOUNT = 8000000;

                data_size = (SAMPLECOUNT * CHANNELCOUNT * BYTESPERSAMPLE);
                buffer_size = (data_size + 0xfff) & ~0xfff;

                // Allocate the buffer.
                out_buf_data = memalign(0x1000, buffer_size);

                gme_play( emu, SAMPLECOUNT, out_buf_data );

                // Prepare the audio data source buffer.
                audout_buf.next = NULL;
                audout_buf.buffer = out_buf_data;
                audout_buf.buffer_size = buffer_size;
                audout_buf.data_size = data_size;
                audout_buf.data_offset = 0;
               
                // Play the buffer.
                rc = audoutAppendAudioOutBuffer(&audout_buf);
                while(1==1) {

                }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
    
    // Stop audio playback.
    rc = audoutStopAudioOut();
    printf("audoutStopAudioOut() returned 0x%x\n", rc);

    // Terminate the default audio output device.
    audoutExit();
    
    gfxExit();
    return 0;
}
