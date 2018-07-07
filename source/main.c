#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <malloc.h>
#include <math.h>

#include <switch.h>
#include <limits.h>

#include "gme.h"

#include "testaudio.h"

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

u32 data_size;
u32 buffer_size;

short int* out_buf_data;

Result rc = 0;

int ccc = 0;
u32 released_out_count;

u64 _t;
int current_track = 0;
int play_testfile = 0;
int track_count = 99;

gme_err_t gme_load_custom( gme_t*, gme_reader_t, long file_size, void* your_data );


int cursor_y = 0;
char current_name[256] = "";

int dirsize = -1;

void print_dir() {
    consoleClear();
    int ii = 0;
    int ii2 = 0;
    dir = opendir(""); //Open current-working-directory.
    if(dir==NULL)
    {
        printf("Failed to open dir.\n");
    }
    else
    {
        if (dirsize == -1) {
            while ((ent = readdir(dir)))
            {
                ii2++;
            }
            
            dirsize = ii2;
            ii2 = 0;
        }

        closedir(dir);

        if (cursor_y>0) printf("^^^\n");
        else printf("---\n");

        dir = opendir(""); //Open current-working-directory.

        while ((ent = readdir(dir)))
        {
            if (ii2 >= cursor_y) {
                if (ii2 == cursor_y) { 
                    printf("> %s\n", ent->d_name); 
                    strcpy(current_name,ent->d_name);
                }
                else printf("  %s\n", ent->d_name);
                ii++;
                if (ii > 10) { printf("vvv\n"); break; }
            }
            ii2++;

        }
        closedir(dir);
    }

    if (dirsize == 0) printf("No files found in current directory.\n");

    gfxFlushBuffers();
    gfxSwapBuffers();
    gfxWaitForVsync();

}

int playing = 0;
unsigned char* buf2;

int offsett = 0;
int breaktime = 1000;
int buffer_inited = 0;

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

        playing = 0;
    
        printf("Sample rate: 0x%x\n", audoutGetSampleRate());
        printf("Channel count: 0x%x\n", audoutGetChannelCount());
        printf("PCM format: 0x%x\n", audoutGetPcmFormat());
        printf("Device state: 0x%x\n", audoutGetDeviceState());

        unsigned char buf[4] = {0};
        size_t bytes = 0, i, readsz = sizeof buf;
        FILE* file;
        size_t bytes2 = 0, filesize = 0;

        if (buffer_inited == 1) { 
            gme_delete(emu);
            free(buf2);
        }

        if (play_testfile == 0) {
            printf("opening file: %s\n", current_name);
            file = fopen(current_name, "rb");
            bytes = fread(buf, 4, 1, file);
            fclose(file);

            file = fopen(current_name, "rb");
            fseek(file, 0, SEEK_END); // seek to end of file
            filesize = ftell(file); // get current file pointer
            fseek(file, 0, SEEK_SET); // seek back to beginning of file
            fclose(file);
            buf2 = malloc(filesize);
        } else {
            printf("playing built-in test song\n");

            buf[0] = testaudio[0]; 
            buf[1] = testaudio[1]; 
            buf[2] = testaudio[2]; 
            buf[3] = testaudio[3];
            buf2 = malloc(testaudio_size);
            filesize = testaudio_size;
        }

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

        if (play_testfile == 0) {
            file = fopen(current_name, "rb");
            bytes2 = fread(buf2, sizeof *buf2, filesize, file);
        } else {
            filesize = testaudio_size;
            memcpy(buf2,testaudio,testaudio_size);
        }

        gme_load_custom (emu, ReaderBlock, filesize, &buf2);

        // Start audio playback.
        if (buffer_inited == 0) { 
            rc = audoutStartAudioOut();
            printf("audoutStartAudioOut() returned 0x%x\n", rc);
        }

        printf("initializing and buffering track...\n");

        gme_start_track( emu, current_track );
        playing = 1;

        buffer_inited = 1;

        timeInitialize();
    }

}

long int SAMPLECOUNT;

void shoulderLogic() {
    printf("track #%d\n",current_track);
    gme_start_track( emu, current_track );
}

int main(int argc, char **argv)
{
    
    gfxInitDefault();

    // Initialize console. Using NULL as the second argument tells the console library to use the internal console structure as current one.
    consoleInit(NULL);

    
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
            play_testfile = 0;
            playtrack();
        }

        if (kDown & KEY_X)
        {
            play_testfile = 1;
            playtrack();
        }
        
        if (kDown & KEY_B)
        {
            printf("Press b\n");
        }
        
        if (kDown & KEY_DUP || kDown & KEY_LSTICK_UP  || kDown & KEY_RSTICK_UP)
        {
            if (cursor_y > 0) cursor_y--;
            print_dir();
        }
        
        if (kDown & KEY_DDOWN  || kDown & KEY_RSTICK_DOWN  || kDown & KEY_RSTICK_DOWN)
        {
            cursor_y++;
            print_dir();
        }
        
        if (kDown & KEY_ZL)
        {
        }
        
        if (kDown & KEY_ZR)
        {
        }
        
        if (R_SUCCEEDED(rc) && playing == 1)
        {
            gme_info_t *info;
            gme_track_info( emu, &info, current_track );

            track_count = gme_track_count(emu);
            printf("track count:%d\n", track_count);

            buffer_size = 20160;

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

            // Play the buffer.
            printf("starting playback on track #%d\n", current_track);
            
            timeGetCurrentTime(TimeType_Default, &_t);
            u64 start_time = _t;

            u64 current_time = 0;
            int willbreak = 0;

            breaktime = info->play_length;
            gme_free_info(info);

            while(1==1) {
                gme_play( emu, buffer_size/2, out_buf_data );
                audout_buf.next = NULL;
                audout_buf.buffer = out_buf_data;
                audout_buf.buffer_size = buffer_size;
                audout_buf.data_size = buffer_size;
                audout_buf.data_offset = 0;

                rc = audoutPlayBuffer(&audout_buf, &released_out_buffer);

                hidScanInput();
                kDown = hidKeysDown(CONTROLLER_P1_AUTO);

                int memlogic = 0;

                if (kDown & KEY_DUP)
                {
                    buffer_size+=0x80;
                    memlogic = 1;
                }
                if (kDown & KEY_DDOWN)
                {
                    buffer_size-=0x80;
                    memlogic = 1;
                }

                if (kDown & KEY_DLEFT)
                {
                    buffer_size-=4;
                    memlogic = 1;
                }
                
                if (kDown & KEY_DRIGHT)
                {
                    buffer_size+=4;
                    memlogic = 1;
                }

                if (kDown & KEY_ZL)
                {
                    buffer_size-=0x100;
                    memlogic = 1;
                }
                
                if (kDown & KEY_ZR)
                {
                    buffer_size+=0x100;
                    memlogic = 1;
                }


                if (kDown & KEY_PLUS)
                {
                    buffer_size+=0x800;
                    memlogic = 1;
                }
                
                if (kDown & KEY_MINUS)
                {
                    buffer_size-=0x800;
                    memlogic = 1;
                }

                if (memlogic == 1) {
                    memlogic = 0;
                    free(out_buf_data);
                    // Allocate the buffer.
                    out_buf_data = memalign(0x1000, buffer_size);
                    
                    // Ensure buffers were properly allocated.
                    if (out_buf_data == NULL)
                    {
                        rc = MAKERESULT(Module_Libnx, LibnxError_OutOfMemory);
                        printf("Failed to allocate sample data buffers\n");
                    }
                    
                    if (R_SUCCEEDED(rc)) {
                        memset(out_buf_data, 0, buffer_size);
                    }

                    printf("bufsize: %d\n",buffer_size);
                    gfxFlushBuffers();
                    gfxSwapBuffers();
                    gfxWaitForVsync();

                }

                if (kDown & KEY_L)
                {
                    if (current_track > 0) current_track--;
                    shoulderLogic();
                    start_time = _t;
                }
                
                if ((kDown & KEY_R) || gme_track_ended(emu) == 1)
                {
                    if (current_track+1 < track_count) current_track++;
                    shoulderLogic();
                    start_time = _t;
                }
                
                if (kDown & KEY_B)
                {
                    break;
                }

                if (kDown & KEY_Y)
                {
                    u64 songtime = gme_tell(emu);
                    printf("t:%ld\n",songtime);
                }
            }

            free(out_buf_data);
            rc = audoutStopAudioOut(); 
            playing = 0; 
            current_track = 0; 
            print_dir();
        }

    }
    
    // Stop audio playback.
    rc = audoutStopAudioOut();
    printf("audoutStopAudioOut() returned 0x%x\n", rc);

    // Terminate the default audio output device.
    audoutExit();
    
    gfxExit();
    return 0;
}
