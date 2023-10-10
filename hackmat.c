/*
hackmat - HackRF in MATLAB!
*/


#include <libhackrf/hackrf.h>

#include "mex.h"
#include "matrix.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>
#include <pthread.h>

#define STREAMLENGTH 1<<19


int8_t buf0[STREAMLENGTH];
int8_t buf1[STREAMLENGTH];

int8_t *b0=buf0;
int8_t *b1=buf1;

volatile uint32_t w0=0;
volatile uint32_t w1=0;


hackrf_device *dev=NULL;
int bytes_read=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct args_t{
    char     op;
    double   fs;
    double   fc;
    double   lnagain;
    double   vgagain;
}args_t;

int rx_callback(hackrf_transfer* transfer){
    //fprintf(stderr,"Read %d bytes\n",transfer->valid_length);
    pthread_mutex_lock(&mutex);
    bytes_read+=transfer->valid_length;
    memcpy(b0,transfer->buffer,transfer->valid_length);
    w0=transfer->valid_length;

    uint32_t tw=w0;
    w0 = w1;
    w1 = tw;

    int8_t *tmp = b0;
    b0 = b1;
    b1 = tmp;

    pthread_mutex_unlock(&mutex);

    return 0;
}

//set the parameters: centerfrequency, bandwidth etc.

void printargs(args_t *a){
    mexPrintf("%c %d %f %s\n",a->op,a->fs,a->fc,a->lnagain);
}

//parseargs: dead simple argument parsing to get parameters from the calling matlab object
void parseargs(args_t *args,int nrhs ,const mxArray *prhs[]){
	
    if (nrhs>0){
		args->op = mxArrayToString(prhs[0])[0];
	}

    if (nrhs>2) //if there's more than one arg (op), need to call with all data types (NULL if not needed)
    {
        //args->d0 = *mxGetInt32s(prhs[1]);
        args->fs = *mxGetDoubles(prhs[1]);
        args->fc = *mxGetDoubles(prhs[2]);
        args->lnagain = *mxGetDoubles(prhs[3]);
        args->vgagain = *mxGetDoubles(prhs[4]);
        //args->s0 = mxArrayToString(prhs[4]);
    }
}

//cleanup: clean the allocated memory & vars here if any, call close...
void cleanup(void){
    hackrf_close(dev);
    hackrf_exit();
    dev = NULL;
}


//This function sets up the sdr;
int setup_device(args_t *a)
{
    hackrf_device_list_t *dl;
    int ret;

    if (dev!=NULL){
        cleanup();
    }
    ret = hackrf_init();

    dl = hackrf_device_list();

    ret |= hackrf_open_by_serial(dl->serial_numbers[0],&dev);

    if (ret<0)
    {
        mexPrintf("hackrf_open_by_serial() failed!\n");
        return -1;
    }
    ret |= hackrf_set_sample_rate(dev,a->fs);

    ret |= hackrf_set_freq(dev,a->fc);

    ret |= hackrf_set_vga_gain(dev,a->vgagain);

    ret |= hackrf_set_lna_gain(dev,a->lnagain);
    
    return ret;
}


//Prints out a list of devices for the Matlab user to see available SoapySDR supported devices
void enumerate_devices(){

    hackrf_device_list_t *dl;
    int ret;

    ret = hackrf_init();

    dl = hackrf_device_list();
    mexPrintf("Found %d devices \n",dl->devicecount);
    for(int i=0;i<dl->devicecount;i++){
        mexPrintf("Serial %s\n",dl->serial_numbers[i]);
    }

}

int receive(args_t *a,mxArray *plhs[]){
    
    if (bytes_read==0){
        if (dev!=NULL){
            hackrf_start_rx(dev, rx_callback, NULL);
        }
    }

    while(w1==0);
    pthread_mutex_lock(&mutex);

    plhs[0] = mxCreateNumericMatrix(w1/2,1,mxSINGLE_CLASS,mxCOMPLEX);
    float* outptr = (float *) mxGetComplexSingles(plhs[0]);
    for(int i=0;i<w1;i++){
        outptr[i] = b1[i]/128.0f;
    }

    w1=0;
    pthread_mutex_unlock(&mutex);

    return 0;
}


// TODO, transmit functionality still missing.
int transmit(args_t *a,const mxArray *prhs[]){

    float* inptr = (float *) mxGetComplexSingles(prhs[1]);
    //const mwSize *dim = mxGetDimensions(prhs[1]);

    //mexPrintf("Dims %d\n",*dim);
    //mexPrintf("Dims %d\n",mxGetNumberOfElements(prhs[0]));
    return 0;

}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	args_t args;

    memset((void *) &args,0,sizeof(args_t));
	parseargs(&args,nrhs,prhs);

	switch(args.op){
		case 'e':
			enumerate_devices();
            break;
        case 'd':
            if (setup_device(&args)){
                mexPrintf("Device setup failed\n");
                //exit(-1);
            }
            break;
        case 'r':
            receive(&args,plhs);
            break;
        case 't':
            transmit(&args,prhs);
            break;
        case 'c':
			cleanup();
            break;
        default:
            break;

	}
}