/******************************************************************************
 *
 * Epp.h, Version 1.4.3 Sat 09 Jul 2011
 *
 * ----------------------------------------------------------------------------
 *
 * Epp (Easy particle propagation)
 * Copyright (C) 2011 CancerCare Manitoba
 *
 * The latest version of Epp and additional information are available online at 
 * http://www.physics.umanitoba.ca/~elbakri/epp/
 *
 * Epp is based on the EGSnrc C++ class library which is available online at
 * http://www.irs.inms.nrc.ca/EGSnrc/pirs898/
 *
 * Epp is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.                                       
 *                                                                           
 * Epp is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Jonas Lippuner
 *   CancerCare Manitoba, Department of Medical Physics
 *   675 McDermot Ave, Winnipeg, Manitoba, R3E 0V9, Canada
 *   Email: jonas@lippuner.ca 
 *
 *****************************************************************************/

#ifndef EPP_
#define EPP_

// egs++ dependencies
#include "egs_advanced_application.h"
#include "egs_interface2.h"
#include "egs_functions.h"
#include "egs_input.h"
#include "egs_library.h"
#include "egs_rndm.h"
#include "egs_run_control.h"
#include "egs_base_source.h"
#include "egs_base_geometry.h"
#define EXPLICIT_XYZ
#include "geometry/egs_nd_geometry/egs_nd_geometry.h"
#include "egs_interpolator.h"
#include "egs_scoring.h"

// local dependencies
#include "bmp.c"


#define OPTION_NONE     ((uchar)0x00)     // 0x00
#define OPTION_PRIMARY  ((uchar)(1 << 0)) // 0x01
#define OPTION_MULTIPLE ((uchar)(1 << 1)) // 0x02
#define OPTION_COMPTON  ((uchar)(1 << 2)) // 0x04
#define OPTION_RAYLEIGH ((uchar)(1 << 3)) // 0x08
#define OPTION_TOTAL    ((uchar)(1 << 4)) // 0x10
#define OPTION_ALL      ((uchar)0x1F)     // 0x1F

#define OPTION_DEFAULT_PHOTOUT        OPTION_NONE
#define OPTION_DEFAULT_COUNT          OPTION_ALL
#define OPTION_DEFAULT_IMAGE_COUNT    OPTION_ALL
#define OPTION_DEFAULT_ENERGY         OPTION_NONE
#define OPTION_DEFAULT_IMAGE_ENERGY   OPTION_NONE

#define OUTIDX_PRIMARY   0
#define OUTIDX_MULTIPLE  1
#define OUTIDX_COMPTON   2
#define OUTIDX_RAYLEIGH  3
#define OUTIDX_TOTAL     4
#define OUTIDX_LENGTH    5

// 1 MeV in Jouls, taken from http://physics.nist.gov/cgi-bin/cuu/Value?tevj
#define MeV 1.602176487E-13

// floating point comparison uncertainty in ULPs
#define MAX_ULPS 20

typedef unsigned char uchar;
typedef unsigned int uint;

struct outputPointers {
    double *pCnt[OUTIDX_LENGTH];
    double *pEng[OUTIDX_LENGTH];
};

struct Detector {
    // location of center
    float x, y, z;
    // size of an individual pixel
    float dx, dy;
    // number of pixels
    uint Nx, Ny;
};

struct PhotonInfo {
    // x, y, z positions
    float x, y, z;
    // u, v, w orientation angles (cosines of angles w.r.t. to x, y, z axis)
    float u, v, w;
    // energy
    float e;
    // statistical weight
    float wt;
};

class EppApplication : public EGS_AdvancedApplication {

public:
    EppApplication(int argc, char **argv);
    
    ~EppApplication();

    int initScoring();
    
    void initializeCounters();

    int ausgab(int iarg);
    
    int outputData();

    int readData();

    void resetCounter();

    int addState(istream &data);

    void outputResults();

    void writeRawData(string filePath, double *data);

    void saveBitmap(string filePath, double *data);

    
protected:
    int startNewShower();


private:
    uchar readOptions(string opt, uchar def);
    uchar readOptionsFromInput(EGS_Input *input, string name, uchar def);

    // pointer array to the output files
    FILE *fout[OUTIDX_LENGTH];

    // pointer array to the socring arrays
    outputPointers outPtrs;
    
    // array for scoring dose
    EGS_ScoringArray *dose;
    
    // XYZ-Geometry in which the dose will be scored
    EGS_XYZGeometry *doseGeometry;
    
    // detector
    Detector *detector;
    int numberOfPixels;
    int bufferSize;

    // output options for phot.out files, raw data, bitmaps and counters 
    // (combination of raw data and bitmaps, used internally for counting buffers)
    uchar oP, oC, oE, oIC, oIE, photon_counters, photon_energy;
    // flags to signal that options were specified in command line arguments and thus
    // take precedence over specification in input file
    bool oPspecified, oCspecified, oEspecified, oICspecified, oIEspecified, oDspecified;

    bool scatterCountEnabled, propagationEnabled, doseEnabled;

    uchar outputOptions[OUTIDX_LENGTH];
    string outputSuffix[OUTIDX_LENGTH];
    bool doseTextOutput, doseBinOutput;
    
    // the total weight and number of all particles generated by the source, 
    // needed to scale the end results to the correct values
    double totalWeight;
    /******** IMPORTANT: MUST BE A DOUBLE ********
     there is a round-off problem with floats that causes the variable not to count 
     beyond 8,388,608 (2^23), with a double that limit is at 4,503,599,627,370,496 (2^52) 
     which should be sufficiently large for our purposes
    */
};

#endif

