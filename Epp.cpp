/******************************************************************************
 *
 * Epp.cpp, Version 1.4.3 Sat 09 Jul 2011
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

#include <dirent.h>

#include "Epp.h"

uchar EppApplication::readOptions(string opt, uchar def) {
    uchar o = OPTION_NONE;
    bool optionFound = false;

    if (opt.find("n") != string::npos)
        return OPTION_NONE;
    else if (opt.find("a") != string::npos)
        return OPTION_ALL;
    else {
        for (int i = 0; i < OUTIDX_LENGTH; i++)
        {
            if (opt.find(outputSuffix[i]) != string::npos) {
                optionFound = true;
                o |= outputOptions[i];
            }
        }
    }

    if (optionFound)
        return o;
    else
        return def;
}

uchar EppApplication::readOptionsFromInput(EGS_Input *input, string name, uchar def) {
    string opt;
    if (input->getInput(name, opt) == 0)
        return readOptions(opt, def);
    else
        return def;
}

EppApplication::EppApplication(int argc, char **argv) : EGS_AdvancedApplication(argc, argv) {
    // initialize options and suffix arrays
    outputOptions[OUTIDX_PRIMARY] = OPTION_PRIMARY;
    outputOptions[OUTIDX_MULTIPLE] = OPTION_MULTIPLE;
    outputOptions[OUTIDX_COMPTON] = OPTION_COMPTON;
    outputOptions[OUTIDX_RAYLEIGH] = OPTION_RAYLEIGH;
    outputOptions[OUTIDX_TOTAL] = OPTION_TOTAL;

    outputSuffix[OUTIDX_PRIMARY] = "p";
    outputSuffix[OUTIDX_MULTIPLE] = "m";
    outputSuffix[OUTIDX_COMPTON] = "c";
    outputSuffix[OUTIDX_RAYLEIGH] = "r";
    outputSuffix[OUTIDX_TOTAL] = "t";

    // read options
    oPspecified = false;
    oCspecified = false;
    oEspecified = false;
    oICspecified = false;
    oIEspecified = false;
    oDspecified = false;
    
    // set default values
    doseTextOutput = false;
    doseBinOutput = false;
    
    string optStr;
    if (getArgument(argc, argv, "-oP", "--output-photout", optStr)) {
        oP = readOptions(optStr, OPTION_DEFAULT_PHOTOUT);
        oPspecified = true;
    }
    else
        oP = OPTION_DEFAULT_PHOTOUT;

    if (getArgument(argc, argv, "-oC", "--output-count", optStr)) {
        oC = readOptions(optStr, OPTION_DEFAULT_COUNT);
        oCspecified = true;
    }
    else
        oC = OPTION_DEFAULT_COUNT;
        
    if (getArgument(argc, argv, "-oE", "--output-energy", optStr)) {
        oE = readOptions(optStr, OPTION_DEFAULT_ENERGY);
        oEspecified = true;
    }
    else
        oE = OPTION_DEFAULT_ENERGY;
        
    if (getArgument(argc, argv, "-oIC", "--output-img-cnt", optStr)) {
        oIC = readOptions(optStr, OPTION_DEFAULT_IMAGE_COUNT);
        oICspecified = true;
    }
    else
        oIC = OPTION_DEFAULT_IMAGE_COUNT;
        
    if (getArgument(argc, argv, "-oIE", "--output-img-e", optStr)) {
        oIE = readOptions(optStr, OPTION_DEFAULT_IMAGE_ENERGY);
        oIEspecified = true;
    }
    else
        oIE = OPTION_DEFAULT_IMAGE_ENERGY;
    
    if (getArgument(argc, argv, "-oD", "--output-dose", optStr)) {
        if (optStr == "text")
            doseTextOutput = true;
        else if (optStr == "bin")
            doseBinOutput = true;
        else if (optStr == "textbin") {
            doseTextOutput = true;
            doseBinOutput = true;
        }
        oDspecified = true;
    }
    
    // initialize variables
    totalWeight = 0.0;
}

EppApplication::~EppApplication() {
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if (fout[i] != NULL)
            fclose(fout[i]);
        if (outPtrs.pCnt[i] != NULL)
            free(outPtrs.pCnt[i]);
        if (outPtrs.pEng[i] != NULL)
            free(outPtrs.pEng[i]);
    }
    
    if (dose != NULL)
        delete dose;
}

string getOutputPrefix() {
    string outputPrefix = string(the_egsio->output_file);
    size_t egsinpPos = outputPrefix.rfind(".egsinp");
    if (egsinpPos == outputPrefix.length()-7)
        outputPrefix = outputPrefix.substr(0,outputPrefix.length()-7);
    return outputPrefix;
}

int EppApplication::initScoring() {
    // check whether previous output files exist in the output directory
    string outPath = getOutputPrefix();
    char cwd[1024];
    string dirPath = getWorkDir() + '/' + outPath;
    DIR *dir = opendir(dirPath.c_str());
    struct dirent *dp;
    size_t filecount = 0;
    size_t i = 0;
    char **files;

    if (dir != NULL) {
        while ((dp = readdir(dir)) != NULL) {
            string entry(dp->d_name);
            if ((entry.find(outPath + ".phot_") == 0) ||
                (entry.find(outPath + ".count_") == 0) ||
                (entry.find(outPath + ".image_count_") == 0) ||
                (entry.find(outPath + ".energy_") == 0) ||
                (entry.find(outPath + ".image_energy_") == 0) ||
                (entry.find(outPath + ".dose.out") == 0) ||
                (entry.find(outPath + ".3ddose") == 0) ||
                (entry.find(outPath + ".egsdat") == 0) ||
                (entry.find(outPath + ".egslog") == 0) ||
                (entry.find(outPath + ".averageWeight.out") == 0) ||
                (entry.find(outPath + "_w") == 0)) {
                string message = "\n\n*** FATAL ERROR ***\nThe output directory (%s) contains an output file (%s) ";
                message += "from a previous run. Please make sure that the output directory does not contain ";
                message += "any previous output files and launch the application again.\n*******************\n\n";
                egsFatal(message.c_str(), dirPath.c_str(), entry.c_str());
            }
        }
        
        closedir(dir);
    }
 
    // set default values
    scatterCountEnabled = false;
    propagationEnabled = false;
    doseEnabled = false;
 
    // read output options from input file if it has not been specified 
    // with the command line arguments
    EGS_Input *output_options = input->takeInputItem("output options");
    if (output_options) {
        if (!oPspecified)
            oP = readOptionsFromInput(output_options, "photout", OPTION_DEFAULT_PHOTOUT);
        if (!oCspecified)
            oC = readOptionsFromInput(output_options, "count", OPTION_DEFAULT_COUNT);
        if (!oEspecified)
            oE = readOptionsFromInput(output_options, "energy", OPTION_DEFAULT_ENERGY);
        if (!oICspecified)
            oIC = readOptionsFromInput(output_options, "img-cnt", OPTION_DEFAULT_IMAGE_COUNT);
        if (!oIEspecified)
            oIE = readOptionsFromInput(output_options, "img-e", OPTION_DEFAULT_IMAGE_ENERGY);
        if (!oDspecified) {
            string optStr;
            if (output_options->getInput("dose", optStr) == 0) {
                if (optStr == "text")
                    doseTextOutput = true;
                else if (optStr == "bin")
                    doseBinOutput = true;
                else if ((optStr == "textbin") || (optStr == "bintext")) {
                    doseTextOutput = true;
                    doseBinOutput = true;
                }
            }
        }
    }
    
    delete output_options;
    
    // specifiy which photon categories should be counted on the detector
    photon_counters = oC | oIC;
    photon_energy = oE | oIE;
    scatterCountEnabled = (photon_counters | photon_energy | oP) > 0;
    
    // if counters is greater than 0, the user wants propagation to the detector
    if ((photon_counters > 0) || (photon_energy)) {
        // read detector data from input
        EGS_Input *detector_input = input->takeInputItem("detector definition");
        if (!detector_input) {
            string message = "\n\n*** FATAL ERROR ***\nPropagation of photons to the detector ";
            message += "was requested, but the input file does not contain a detector definition. ";
            message += "Please either provide a detector definition in the input file, or turn off ";
            message += "the raw data and bitmap output files.\n*******************\n\n";
            egsFatal(message.c_str());
        }

        detector = new Detector();    

        vector<EGS_Float> position;
        if (detector_input->getInput("position", position) == 0) {
            detector->x = position[0];
            detector->y = position[1];
            detector->z = position[2];
        }
        else
            egsFatal("\n\n*** FATAL ERROR ***\n\"position\" in the detector definition could not be read or does not exist.\n*******************\n\n");

        vector<int> size;
        if (detector_input->getInput("size", size) == 0) {
            detector->Nx = size[0];
            detector->Ny = size[1];
        }
        else
            egsFatal("\n\n*** FATAL ERROR ***\n\"size\" in the detector definition could not be read or does not exist.\n*******************\n\n");

        vector<EGS_Float> pixelSize;
        if (detector_input->getInput("pixel size", pixelSize) == 0) {
            detector->dx = pixelSize[0];
            detector->dy = pixelSize[1];
        }
        else
            egsFatal("\n\n*** FATAL ERROR ***\n\"pixel_size\" in the detector definition could not be read or does not exist.\n*******************\n\n");

        delete detector_input;

        propagationEnabled = true;
        numberOfPixels = detector->Nx * detector->Ny;
        bufferSize = numberOfPixels * sizeof(double);
    }
    else {
        detector = NULL;
        propagationEnabled = false;
        numberOfPixels = 0;
        bufferSize = 0;
    }
    
    // if the user wants dose output
    if (doseTextOutput || doseBinOutput) {
        // read dose calculation data from input
        EGS_Input *dose_input = input->takeInputItem("dose calculation");
        if (!dose_input) {
            string message = "\n\n*** FATAL ERROR ***\nDose calculation ";
            message += "was requested, but the input file does not contain a dose calculation input. ";
            message += "Please either provide a dose calculation input in the input file, or turn off ";
            message += "the dose output.\n*******************\n\n";
            egsFatal(message.c_str());
        }

        string phantomName;
        if (dose_input->getInput("phantom", phantomName) != 0)
            egsFatal("\n\n*** FATAL ERROR ***\n\"phantom\" in the dose calculation input could not be read or does not exist.\n*******************\n\n");
        
        EGS_BaseGeometry *phantomGeometry = geometry->getGeometry(phantomName);

        if (phantomGeometry == NULL) {
            string message = "\n\n*** FATAL ERROR ***\nThe phantom \"";
            message += phantomName;
            message += "\" for the dose calculation does not exist.\n*******************\n\n";
            egsFatal(message.c_str());
        }
        
        if (phantomGeometry->getType() != "EGS_XYZGeometry") {
            string message = "\n\n*** FATAL ERROR ***\nThe phantom \"";
            message += phantomName;
            message += "\" for the dose calculation is not an EGS_XYZGeometry.\n*******************\n\n";
            egsFatal(message.c_str());
        }
        
        doseGeometry = (EGS_XYZGeometry*)phantomGeometry;
        
        doseEnabled = true;
        dose = new EGS_ScoringArray(geometry->regions() + 1);
    }
    else {
        doseGeometry = NULL;
        doseEnabled = false;
        dose = NULL;
    }
    
    // open photon output files if photout is enabled
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((oP & outputOptions[i]) > 0) {
            string ext = ".phot_";
            ext += outputSuffix[i];
            ext += ".out";
            string path = constructIOFileName(ext.c_str(), true);
            fout[i] = fopen(path.c_str(), "wb");
        }
        else
            fout[i] = NULL;
    }

    // allocate memory for counting photons if count or count image output is enabled
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_counters & outputOptions[i]) > 0)
            outPtrs.pCnt[i] = (double*)malloc(bufferSize);
        else
            outPtrs.pCnt[i] = NULL;
    }
    
    // allocate memory for scoring photon energies if energy or energy image output is enabled
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_energy & outputOptions[i]) > 0)
            outPtrs.pEng[i] = (double*)malloc(bufferSize);
        else
            outPtrs.pEng[i] = NULL;
    }

    // initialize counters to 0
    initializeCounters();

    // set ausgab call to false for everything by default
    for (int i = 0; i < UnknownCall; i++)
        setAusgabCall((AusgabCall)i, false);

    // if we want to calculate dose, we need ausgab being called for
    // BeforeTransport, EgsCut, PegsCut, UserDiscard, ExtraEnergy
    if (doseEnabled) {
        setAusgabCall(BeforeTransport, true);
        setAusgabCall(EgsCut, true);
        setAusgabCall(PegsCut, true);
        setAusgabCall(UserDiscard, true);
        setAusgabCall(ExtraEnergy, true);
    }

    // if we want to count scatter events, we need ausgab being called for
    // UserDiscard, AfterBrems, AfterCompton, AfterReyleigh
    if (scatterCountEnabled) {
        setAusgabCall(UserDiscard, true);
        setAusgabCall(AfterBrems, true);
        setAusgabCall(AfterCompton, true);
        setAusgabCall(AfterRayleigh, true);
    }

    return 0;
}

void EppApplication::initializeCounters() {
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if (outPtrs.pCnt[i] != NULL) {
            for (int p = 0; p < numberOfPixels; p++)
                outPtrs.pCnt[i][p] = 0.0;
        }
    }
    
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if (outPtrs.pEng[i] != NULL) {
            for (int p = 0; p < numberOfPixels; p++)
                outPtrs.pEng[i][p] = 0.0;
        }
    }
    
    if (doseEnabled)
        dose->reset();
}

// taken from http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
bool almostEqual(float A, float B)
{
    // Make sure maxUlps is non-negative and small enough that the
    // default NAN won't compare as equal to anything.
    //assert(maxUlps > 0 && maxUlps < 4 * 1024 * 1024);

    // Make aInt lexicographically ordered as a twos-complement int
    int aInt = *(int*)&A;
    if (aInt < 0)
        aInt = 0x80000000 - aInt;

    // Make bInt lexicographically ordered as a twos-complement int
    int bInt = *(int*)&B;
    if (bInt < 0)
        bInt = 0x80000000 - bInt;

    int intDiff = abs(aInt - bInt);
    if (intDiff <= MAX_ULPS)
        return true;

    return false;
}


int EppApplication::ausgab(int iarg) {
    /******************************* About iarg *****************************
     *                                                                      *
     * iarg describes the situation under which ausgab is being called,     *
     * the relevant values of iarg for this application are:                *
     *                                                                      *
     * UserDiscard (3)                                                      *
     * --> photon is going to be discarded because it's leaving the volume  *
     *                                                                      *
     * AfterBrems (7)                                                       *
     * --> a bremsstrahlung interaction has just occurred                   *
     *                                                                      *
     * AfterCompton (18)                                                    *
     * --> photon has just undergone a Compton scatter event                *
     *                                                                      *
     * AfterRayleigh (24)                                                   *
     * --> photon has just undergone a Rayleigh scatter event               *
     *                                                                      *
     ************************************************************************/

    /********************************** NOTE ********************************
     *                                                                      *
     * 1 needs to be subtracted from all indices (np, npold) because array  *
     * numbering in C starts at 0 and not at 1 as it does in Fortran        *
     *                                                                      *
     ************************************************************************/
    int i;
    int np = the_stack->np - 1;
    int npOld = the_stack->npold - 1;

    // score the dose if it is enabled
    if (doseEnabled && (iarg <= 4)) {
        int ir = the_stack->ir[np] - 1;
        dose->score(ir, the_epcont->edep * the_stack->wt[np]);    
    }

    if (scatterCountEnabled) {
        // track the number of scatter events experienced by photon:
        // if a bremsstrahlung interaction (iarg = AfterBrems = 7) occurred
        if (iarg == AfterBrems) {
            // the photons created in the bremsstrahlung interaction are
            // between npOld + 1 and np
            // count these photons under multiple scattered, i.e. increase scatter counter by 2
            for (i = npOld + 1; i <= np; i++)
                the_stack->latch[i] += 2;
        }
        // else if a Compton scatter event (iarg = AfterCompton = 18) occurred
        else if (iarg == AfterCompton) {
            // if bound compton scattering is turned on, there is a chance that the 
            // Compton scatter event was rejected, in that case npOld = np, additionally
            // if Roussian Roulette is used it is possible that npOld = np even
            // if the scatter event actually took place but all electrons were 
            // eliminated with Roussian Roulette, if that's the case then i_survived_RR
            // is larger than zero
            // i.e. the Compton scatter event has actually happend only if npOld != np
            // or i_survived_RR > 0, in any case, the photon will be at npOld in the stack
            if ((npOld != np) || (the_egsvr->i_survived_RR > 0))
                the_stack->latch[npOld] += 1;
        }
        // else if a Rayleigh scatter event (iarg = AfterRayleigh = 24) occured
        else if (iarg == AfterRayleigh) {
            // the photon is at npOld
            //the_stack->latch[npOld] += 1 << 16;
            for (i = npOld; i <= np; i++)
                the_stack->latch[i] += 1 << 16;
        }
        // else if the particle is being discarded (iarg = UserDiscard = 3)  
        // and if it is a photon (i.e. charge = iq(np) = 0)
        // and if it is in the region with index 1 (actually index 0 in C-style, which is defined as outside)
        // and if it is not going perpendicular to the z direction (i.e. w != 0.0)
        // then write the photon to the appropriate output file and count it on the detector
        else if ((iarg == UserDiscard) && (the_stack->iq[np] == 0) && (the_stack->ir[np] == 1) &&
                 (the_stack->w[np] != 0.0)) {
            // read the number of Compton and Rayleigh scatter events from latch,
            // because we split the 32-bit integer latch in two parts to count Compton
            // and Rayleigh scattering seperately, each count has therefore only 16 bits,
            // which makes it an unsigined short (0..65,535)

            // read photon information from stack
            PhotonInfo phot;
            phot.x = the_stack->x[np];
            phot.y = the_stack->y[np];
            phot.z = the_stack->z[np];
            phot.u = the_stack->u[np];
            phot.v = the_stack->v[np];
            phot.w = the_stack->w[np];
            phot.e = the_stack->E[np];
            phot.wt = the_stack->wt[np];

            // for Compton scatter events latch was increased by 1 so we need to find the
            // remainder of latch divided by 2^16. For performance reasons we don't use
            // the modulo operator but rather perform a bitwise AND with (2^16 - 1)
            unsigned short nCompton = the_stack->latch[np] & (1 << 16) - 1;

            // for Rayleigh scatter events latch was increased by 2^16 so we need to divide
            // latch by 2^16 (integer division). The easiest (and fastest) way to do this
            // is to right shift latch by 16 bits
            unsigned short nRayleigh = the_stack->latch[np] >> 16;

            // the following conditional statements are optimized for the case where
            // p > m > c > r (where p, m, c and r are the number of primary, multiple scattered, 
            // compton scattered and rayleigh scattered photons respectively)

            // index for the appropriate output file and socring arrays
            uchar outIdx;

            // primary photons never been scattered
            if (nCompton + nRayleigh == 0)
                outIdx = OUTIDX_PRIMARY;
            // multiple scattered photons have been scattered more than once
            else if (nCompton + nRayleigh > 1)
                outIdx = OUTIDX_MULTIPLE;
            // Compton scattered photons have been Compton scattered exactly once
            // and have never been Rayleigh scattered
            else if (nCompton == 1)
                outIdx = OUTIDX_COMPTON;
            // Rayleigh scattered photons have been Rayleigh scattered exactly once
            // and have never been Compton scattered
            else
                outIdx = OUTIDX_RAYLEIGH;

            // write the photon information to the output file if it is enabled
            if (fout[outIdx] != NULL)
                fwrite(&phot, sizeof(PhotonInfo), 1, fout[outIdx]);
            if (fout[OUTIDX_TOTAL] != NULL)
                fwrite(&phot, sizeof(PhotonInfo), 1, fout[OUTIDX_TOTAL]);

            // calculate distance to detector
            float zDiff = detector->z - phot.z;
            float t = zDiff / phot.w;
            
            // only propagate to detector if propagation enabled and detector
            // is in direction of travel of the photon
            if (propagationEnabled && (t > 0.0))  {
                // propagate photon to the detector
                float posX = phot.x + t * phot.u;
                float posY = phot.y + t * phot.v;
            
                /*** OLD METHOD ***
                int pixelX = (int)round(((posX - detector->x) / detector->dx + detector->Nx / 2));
                int pixelY = (int)round(((posY - detector->y) / detector->dy + detector->Ny / 2));

                double countValue = (double)the_stack->wt[np];
                double energyValue = (double)the_stack->wt[np] * (double)the_stack->E[np];

                // check whether pixel is on the detector
                if ((pixelX >= 0) && (pixelX < detector->Nx) &&
                    (pixelY >= 0) && (pixelY < detector->Ny)) {
                    int detectorIdx = pixelX + detector->Nx * pixelY;
                    
                    // score the photon in the appropriate scoring array if it is enabled
                    if (outPtrs.pCnt[outIdx] != NULL)
                        outPtrs.pCnt[outIdx][detectorIdx] += countValue;
                    if (outPtrs.pCnt[OUTIDX_TOTAL] != NULL)
                        outPtrs.pCnt[OUTIDX_TOTAL][detectorIdx] += countValue;
                        
                    if (outPtrs.pEng[outIdx] != NULL)
                        outPtrs.pEng[outIdx][detectorIdx] += energyValue;
                    if (outPtrs.pEng[OUTIDX_TOTAL] != NULL)
                        outPtrs.pEng[OUTIDX_TOTAL][detectorIdx] += energyValue;
                }
                */

                int pixelX, pixelY;

                float xIdx = (posX - detector->x) / detector->dx + (float)detector->Nx / (float)2;
                float yIdx = (posY - detector->y) / detector->dy + (float)detector->Ny / (float)2;

                int split = 0;
                
                float lowerX = floor(xIdx);
                float upperX = ceil(xIdx);
                
                float lowerY = floor(yIdx);
                float upperY = ceil(yIdx);
                
                if (almostEqual(xIdx, lowerX)) {
                    split += 1;
                    pixelX = (int)lowerX;
                } else if (almostEqual(xIdx, upperX)) {
                    split += 1;
                    pixelX = (int)upperX;
                }
                else
                    pixelX = (int)lowerX;
                    
                if (almostEqual(yIdx, lowerY)) {
                    split += 2;
                    pixelY = (int)lowerY;
                } else if (almostEqual(yIdx, upperY)) {
                    split += 2;
                    pixelY = (int)upperY;
                }
                else
                    pixelY = (int)lowerY;

                double countValue = (double)the_stack->wt[np];
                double energyValue = (double)the_stack->wt[np] * (double)the_stack->E[np];
                
                int numPixels;
                int xIndices[4];
                int yIndices[4];

                xIndices[0] = pixelX;
                yIndices[0] = pixelY;

                switch (split) {
                    case 0:
                        numPixels = 1;
                        break;
                    
                    case 1:
                        numPixels = 2;
                        xIndices[1] = pixelX - 1;
                        yIndices[1] = pixelY;
                        countValue /= (double)2;
                        energyValue /= (double)2;
                        break;
                    
                    case 2:
                        numPixels = 2;
                        xIndices[1] = pixelX;
                        yIndices[1] = pixelY - 1;
                        countValue /= (double)2;
                        energyValue /= (double)2;
                        break;
                    
                    case 3:
                        numPixels = 4;
                        xIndices[1] = pixelX - 1;
                        yIndices[1] = pixelY;
                        xIndices[2] = pixelX;
                        yIndices[2] = pixelY - 1;
                        xIndices[3] = pixelX - 1;
                        yIndices[3] = pixelY - 1;
                        countValue /= (double)4;
                        energyValue /= (double)4;
                        break;
                }
                
                
                for (int p = 0; p < numPixels; p++) {
                    int x = xIndices[p];
                    int y = yIndices[p];
                    
                    // check whether pixel is on the detector
                    if ((x >= 0) && (x < detector->Nx) &&
                        (y >= 0) && (y < detector->Ny)) {
                        int detectorIdx = x + detector->Nx * y;
                        
                        // score the photon in the appropriate scoring array if it is enabled
                        if (outPtrs.pCnt[outIdx] != NULL)
                            outPtrs.pCnt[outIdx][detectorIdx] += countValue;
                        if (outPtrs.pCnt[OUTIDX_TOTAL] != NULL)
                            outPtrs.pCnt[OUTIDX_TOTAL][detectorIdx] += countValue;
                            
                        if (outPtrs.pEng[outIdx] != NULL)
                            outPtrs.pEng[outIdx][detectorIdx] += energyValue;
                        if (outPtrs.pEng[OUTIDX_TOTAL] != NULL)
                            outPtrs.pEng[OUTIDX_TOTAL][detectorIdx] += energyValue;
                    }
                }
            }
        }
    }
}

int EppApplication::outputData() {
    // We first call the outputData() function of our base class. 
    // This takes care of saving data related to the source, the random 
    // number generator, CPU time used, number of histories, etc.
    int err = EGS_AdvancedApplication::outputData();
    if( err ) return err;
    
    // We then write our own data to the data stream. data_out is 
    // a pointer to a data stream that has been opened for writing 
    // in the base class.

    data_out->write((char*)(&totalWeight), sizeof(double));
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_counters & outputOptions[i]) > 0)
            data_out->write((char*)outPtrs.pCnt[i], bufferSize);
    }
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_energy & outputOptions[i]) > 0)
            data_out->write((char*)outPtrs.pEng[i], bufferSize);
    }
    if (doseEnabled)
        dose->storeState(*data_out);

    return 0;
}

int EppApplication::readData() {
    // We first call the readData() function of our base class.
    // This takes care of reading data related to the source, the random
    // number generator, CPU time used, number of histories, etc. 
    // (everything that was stored by the base class outputData() method).
    int err = EGS_AdvancedApplication::readData();
    if( err ) return err;
    
    // We then read our own data from the data stream.
    // data_in is a pointer to an input stream that has been opened 
    // by the base class.
    
    // there is one byte after the current pointer before our data starts
    data_in->get();
    data_in->read((char*)(&totalWeight), sizeof(double));
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_counters & outputOptions[i]) > 0)
            data_in->read((char*)outPtrs.pCnt[i], bufferSize);
    }
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_energy & outputOptions[i]) > 0)
            data_in->read((char*)outPtrs.pEng[i], bufferSize);
    }
    if (doseEnabled)
        dose->setState(*data_in);

    return 0;
}

void EppApplication::resetCounter() {
    // Reset everything in the base class
    EGS_AdvancedApplication::resetCounter();
    
    // Reset our own data to zero.
    totalWeight = 0.0;
    initializeCounters();
}

int EppApplication::addState(istream &data) {
    // Call first the base class addState() function to read and add 
    // all data related to source, RNG, CPU time, etc. 
    int err = EGS_AdvancedApplication::addState(data);
    if( err ) return err;

    // Then read our own data to temporary variables and add to 
    // our results.

    // allocate temporary memory
    double tmpTotalWeight;
    double *tmp = (double*)malloc(bufferSize);

    // read the raw data of all enabled counters and add them to the 
    // existing counters
    
    // there is one byte after the current pointer before our data starts
    data.get();
    data.read((char*)(&tmpTotalWeight), sizeof(double));
    totalWeight += tmpTotalWeight;
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_counters & outputOptions[i]) > 0) {
            data.read((char*)tmp, bufferSize);
            for (int p = 0; p < numberOfPixels; p++)
                outPtrs.pCnt[i][p] += tmp[p];
        }
    }
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((photon_energy & outputOptions[i]) > 0) {
            data.read((char*)tmp, bufferSize);
            for (int p = 0; p < numberOfPixels; p++)
                outPtrs.pEng[i][p] += tmp[p];
        }
    }
    
    if (doseEnabled) {
        EGS_ScoringArray tmpDose(geometry->regions() + 1);
        tmpDose.setState(data);
        (*dose) += tmpDose;
    }

    // free temporary memory
    free(tmp);
    return 0;
}

void EppApplication::outputResults() {
    // scale the data with the average weight to get the correct values
    double averageWeight = 0.0;
    if (totalWeight > 0) {
        averageWeight = totalWeight / (double) run->getNcase();
        for (int i = 0; i < OUTIDX_LENGTH; i++) {
            if ((photon_counters & outputOptions[i]) > 0) {
                for (int p = 0; p < numberOfPixels; p++)
                    outPtrs.pCnt[i][p] /= averageWeight;
            }
        }
        for (int i = 0; i < OUTIDX_LENGTH; i++) {
            if ((photon_energy & outputOptions[i]) > 0) {
                for (int p = 0; p < numberOfPixels; p++)
                    outPtrs.pEng[i][p] /= averageWeight;
            }
        }
    }
    
    string outputPrefix = getOutputPrefix();
    
    if (doseEnabled) {
        EGS_Vector pos = EGS_Vector(
            (doseGeometry->getXPositions()[0] + doseGeometry->getXPositions()[doseGeometry->getNx()]) / 2.0, 
            (doseGeometry->getYPositions()[0] + doseGeometry->getYPositions()[doseGeometry->getNy()]) / 2.0, 
            (doseGeometry->getZPositions()[0] + doseGeometry->getZPositions()[doseGeometry->getNz()]) / 2.0);
        int offset = geometry->isWhere(pos) - doseGeometry->isWhere(pos) + 1;

        int nreg = doseGeometry->regions();
        double *doseValues = (double*)malloc(nreg * sizeof(double));
        double *doseErrors = (double*)malloc(nreg * sizeof(double));

        for (int r = 0; r < nreg; r++) {
            int med = doseGeometry->medium(r);
            EGS_Float realRho = doseGeometry->getRelativeRho(r)*the_media->rho[med];
            double e, de;
            dose->currentResult(r + offset, e, de);            
            doseErrors[r] = e > 0 ? de / e : 1;
            doseValues[r] = e * ((double)current_case * MeV / (doseGeometry->getVolume(r) * realRho * 1e-3 * averageWeight));
        }
        
        if (doseTextOutput) {
            FILE *doseText = fopen((outputPrefix + ".3ddose").c_str(), "w");
            
            fprintf(doseText, "  %d  %d  %d\n", doseGeometry->getNx(), doseGeometry->getNy(), doseGeometry->getNz());

            for (int x = 0; x <= doseGeometry->getNx(); x++)
                fprintf(doseText, "  %10.6f", doseGeometry->getXPositions()[x]);
            fprintf(doseText, "\n");
            
            for (int y = 0; y <= doseGeometry->getNy(); y++)
                fprintf(doseText, "  %10.6f", doseGeometry->getYPositions()[y]);
            fprintf(doseText, "\n");
            
            for (int z = 0; z <= doseGeometry->getNz(); z++)
                fprintf(doseText, "  %10.6f", doseGeometry->getZPositions()[z]);
            fprintf(doseText, "\n");
            
            for (int r = 0; r < nreg; r++)
                fprintf(doseText, "  %21.15E", doseValues[r]);
            fprintf(doseText, "\n");
            
            for (int r = 0; r < nreg; r++)
                fprintf(doseText, "  %21.15E", doseErrors[r]);
            fprintf(doseText, "\n");

            fclose(doseText);
        }
        
        if (doseBinOutput) {
            FILE *doseBin = fopen((outputPrefix + ".dose.out").c_str(), "wb");

            int dim[3];
            dim[0] = doseGeometry->getNx();
            dim[1] = doseGeometry->getNy();
            dim[2] = doseGeometry->getNz();
            fwrite(dim, sizeof(int), 3, doseBin);
            
            fwrite(doseGeometry->getXPositions(), sizeof(double), doseGeometry->getNx() + 1, doseBin);
            fwrite(doseGeometry->getYPositions(), sizeof(double), doseGeometry->getNy() + 1, doseBin);
            fwrite(doseGeometry->getZPositions(), sizeof(double), doseGeometry->getNz() + 1, doseBin);

            fwrite(doseValues, sizeof(double), nreg, doseBin);
            fwrite(doseErrors, sizeof(double), nreg, doseBin);

            fclose(doseBin);
        }
    }

    // write average weight to output file if any photout file is enabled
    if ((oP & OPTION_ALL) > 0) {
        FILE *avgWt = fopen((outputPrefix + ".averageWeight.out").c_str(), "wb");
        float avgWtF = (float)averageWeight;
        fwrite(&avgWtF, sizeof(float), 1, avgWt);
        fclose(avgWt);
    }

    // write count and energy output data
    for (int i = 0; i < OUTIDX_LENGTH; i++) {
        if ((oC & outputOptions[i]) > 0)
            writeRawData(outputPrefix + ".count_" + outputSuffix[i] + ".out", outPtrs.pCnt[i]);
        if ((oE & outputOptions[i]) > 0)
            writeRawData(outputPrefix + ".energy_" + outputSuffix[i] + ".out", outPtrs.pEng[i]);
        if ((oIC & outputOptions[i]) > 0)
            saveBitmap(outputPrefix + ".image_count_" + outputSuffix[i] + ".bmp", outPtrs.pCnt[i]);
        if ((oIE & outputOptions[i]) > 0)
            saveBitmap(outputPrefix + ".image_energy_" + outputSuffix[i] + ".bmp", outPtrs.pEng[i]);
    }
}

int EppApplication::startNewShower() {
    totalWeight += p.wt;
    
    if( current_case != last_case ) {
        if (doseEnabled)
            dose->setHistory(current_case); 
        last_case = current_case;
    }
 
    return 0;
}

void EppApplication::writeRawData(string filePath, double *data) {
    float *dataF = (float*)malloc(numberOfPixels * sizeof(float));
    for (int p = 0; p < numberOfPixels; p++)
        dataF[p] = (float)data[p];
    
    FILE *f = fopen(filePath.c_str(), "wb");
    fwrite(&detector->Nx, sizeof(int), 1, f);
    fwrite(&detector->Ny, sizeof(int), 1, f);
    fwrite(dataF, sizeof(float), numberOfPixels, f);
    fclose(f); 
    
    free(dataF);
}

void EppApplication::saveBitmap(string filePath, double *data) {
    // find highest photon count
    double highestCount = data[0];
    int i;
    for (i = 0; i < numberOfPixels; i++) {
        if (data[i] > highestCount)
            highestCount = data[i];
    }

    // calculate rgb values
    char *rgb = (char*)malloc(3 * numberOfPixels);
    for (i = 0; i < numberOfPixels; i++) {
        char val = char (data[i] / highestCount * 255);
        rgb[3 * i] = val;
        rgb[3 * i + 1] = val;
        rgb[3 * i + 2] = val;
    }

    // write bitmap
    write_bmp(filePath.c_str(), detector->Nx, detector->Ny, rgb);

    free(rgb);
}

int main(int argc, char** argv) {
    EppApplication app(argc, argv);

    int err = app.initSimulation();
    
    if (err != 0) 
        return err;
    
    err = app.runSimulation();
    
    if (err != 0) 
        return err;
    
    return app.finishSimulation();
}

