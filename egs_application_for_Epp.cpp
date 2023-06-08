/******************************************************************************
 *
 * egs_application_for_Epp.cpp, Version 1.4.3 Sat 09 Jul 2011
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

 
/******************************************************************************
 *
 * THIS IS A MODIFIED VERSION OF THE FILE egs_application.cpp DISTRIBUTED WITH 
 * THE EGSNRC C++ CLASS LIBRARY.
 *
 * The following changes have been made to extend the input file for Epp with
 * the capability of referencing other files and using *.egsphant files:
 *
 *  all occurences of           have been replaced with     # of occurences
 * -------------------         -------------------------   -----------------
 *  EGS_Application             EGS_Application             51
 *  active_egs_application      active_egs_application      10
 *  EGS_RunControl              EGS_RunControl              2
 *
 *
 * Additional modifications to the original code are indicated below.
 *
 *****************************************************************************/

 
/*============================================================================*
 *                                                                            *
 *                         ORIGINAL FILE STARTS HERE                          *
 *                                                                            *
 *============================================================================*/


/******************************************************************************
 *
 *  $Id: egs_application.cpp,v 1.17 2009/08/09 12:49:28 iwan Exp $
 *
 * ----------------------------------------------------------------------------
 *
 * EGSnrc C++ class library egspp
 * Copyright (C) 2004-2005 Iwan Kawrakow, National Research Council of Canada
 *                                                                            
 * egspp is free software; you can redistribute it and/or modify      
 * it under the terms of the GNU General Public License as published by      
 * the Free Software Foundation; either version 2 of the License, or         
 * (at your option) any later version.                                       
 *                                                                           
 * egspp is distributed in the hope that it will be useful,           
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             
 * GNU General Public License for more details.                              
 *                                                                           
 * You should have received a copy of the GNU General Public License         
 * along with this program; if not, write to the Free Software               
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 
 *
 * Individuals and entities not willing to embrace the open source idea 
 * can license this software under a closed source license by contacting 
 * Iwan Kawrakow.
 *
 * ----------------------------------------------------------------------------
 *
 *   Contact:
 *
 *   Iwan Kawrakow
 *   Ionizing Radiation Standards, Institute for National Measurement Standards
 *   National Research Council of Canada, Ottawa, ON, K1A 0R6, Canada
 *   voice: ++1 613 993 2197 ext 241
 *   fax:   ++1 613 952 9865
 *   email: iwan@irs.phy.nrc.ca 
 *
 ****************************************************************************/

/*! \file egs_application.cpp
 *  \brief EGS_Application implementation
 *  \IK
 */

/***** CODE MODIFICATION: REPLACED *****
Jonas Lippuner, 2010-04-29

#include "egs_application.h"

 ***** WITH *****/

#include "egs_application_for_Epp.h"

/***** END OF CODE MODIFICATION *****/

#include "egs_functions.h"
#include "egs_input.h"
#include "egs_base_source.h"
#include "egs_rndm.h"
#include "egs_run_control.h"
#include "egs_base_source.h"
#include "egs_simple_container.h"
#include "egs_ausgab_object.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <fstream>

using namespace std;

#ifdef WIN32
const char fs = 92;
#define F_OK 0
#define W_OK 2
#define R_OK 4
#include <io.h>
#define EGS_ACCESS ::_access
#else
const char fs = '/';
#include <unistd.h>
#define EGS_ACCESS ::access
#endif

static char __egs_app_msg1[] = "EGS_Application::EGS_Application(int,char**):";
static char __egs_app_msg2[] = "EGS_Application::initSimulation():";
static char __egs_app_msg3[] = "EGS_Application::runSimulation():";

static EGS_LOCAL bool __egs_find_pegsfile(const vector<string> &paths, 
        const string &pegs_file, string &abs_pegs_file) {
    string pfile = pegs_file;
    if( pfile.find(".pegs4dat") == string::npos ) pfile += ".pegs4dat";
    for(unsigned int j=0; j<paths.size(); j++) {
        string this_pegs = egsJoinPath(paths[j],pfile);
        if( !EGS_ACCESS(this_pegs.c_str(),R_OK) ) {
            abs_pegs_file = this_pegs; return true;
        }
    }
    return false;
}

static EGS_LOCAL EGS_Application *active_egs_application = 0;

int EGS_Application::n_apps = 0;

EGS_EXPORT EGS_Application *EGS_Application::activeApplication() {
    return active_egs_application;
}

EGS_EXPORT void EGS_Application::setActiveApplication(EGS_Application *a) {
    active_egs_application = a;
};

int EGS_Application::userScoring(int iarg) {
    if( a_objects ) {
        int early_return = 0;
        for(int j=0; j<a_objects[iarg].size(); ++j) {
            int res = a_objects[iarg][j]->processEvent((AusgabCall)iarg);
            if( res < 0 ) return res;
            if( res > 0 ) early_return = res;
        }
        if( early_return > 0 ) return early_return;
    }
    return ausgab(iarg);
}

void EGS_Application::checkEnvironmentVar(int &argc, char **argv, 
        const char *n1, const char *n2, const char *env, string &var) {
    char *aux = getenv(env);
    if( aux ) var = aux; else var = "";
    getArgument(argc,argv,n1,n2,var);
    if( !var.size() ) egsFatal("%s\n  the environment variable %s is not set"
           " and it was not passed as argument\n",__egs_app_msg1,env);
    int n = var.size()-1;
    if( var[n] != '/' && var[n] != fs ) var += fs;
}

/***** CODE MODIFICATION: ADDED *****/
// Jonas Lippuner, 2009-07-21
// Jonas Lippuner, 2009-08-31 modified to add support for *.egsphant files with #egsphant directive
// Jonas Lippuner, 2009-11-20 removed XML spport and went back to original *.egsinp input file

string getDir(string path) {
    string dir;
    size_t pos = path.find_last_of('/');
    if (pos == string::npos)
        return "";
    else
        return path.substr(0, pos);
}

string Trim(string str) {
    string trimmed;
    for (int i = 0; i < str.length(); i++) {
        char c = str[i];
        if (c >= 32)
            trimmed += c;
    }
    return trimmed;
}

vector<string> EGS_Application::PreParseFile(string path, string curDir, FILE *f) {
    vector<string> intermediate_files;
    vector<string>::iterator intermediate_files_iterator;
    
    if (path.find('/') != 0)
        path = curDir + '/' + path;
    
    path = Trim(path);
    
    FILE *in = fopen(path.c_str(), "r");
    if (in != NULL) {
        char buf[1024 * 1024];
        while (fgets(buf, 1024 * 1024, in) != NULL) {
            string line(buf);
            size_t posInclude = line.find("#include");
            size_t posEgsphant = line.find("#egsphant");
            if (posInclude != string::npos) {
                string includePath = line.substr(posInclude + 9);
                PreParseFile(includePath, getDir(path), f);
            }
            else if (posEgsphant != string::npos) {
                string egsphantInfo = line.substr(posEgsphant + 10);
                size_t posFirstSpace = line.find_first_of(' ', posEgsphant + 10);
                if (posFirstSpace == string::npos) {
                    if (f != NULL)
                        fclose(f);
                    fclose(in);
                    egsFatal("\n\n*** FATAL ERROR ***\nInvalid #egsphant directive. Enter #egsphant [name of geometry] [path to the *.egsphant file]\n*******************\n\n");
                }
                else {
                    string phantomName = line.substr(posEgsphant + 10, posFirstSpace - posEgsphant - 10);
                    string egsphantPath = Trim(line.substr(posFirstSpace + 1));
                    if (egsphantPath.find('/') != 0)
                        egsphantPath = getDir(path) + '/' + egsphantPath;
                    
                    FILE *egsphantFile = fopen(egsphantPath.c_str(), "r");
                    if (egsphantFile != NULL) {
                        // looping variable
                        int i;
                        
                        int nMed; // number of media
                        fscanf(egsphantFile, "%d\n", &nMed);

                        // read media names
                        string *media = new string[nMed];
                        for (i = 0; i < nMed; i++) {
                            char mBuf[1024];
                            fgets(mBuf, 1024, egsphantFile);
                            media[i] = Trim(string(mBuf));
                        }

                        // skip next line (contains dummy input)
                        char dummyBuf[1024];
                        fgets(dummyBuf, 1024, egsphantFile);

                        // read voxel numbers
                        int nx, ny, nz;
                        fscanf(egsphantFile, "%d %d %d\n", &nx, &ny, &nz);

                        // read voxel boundaries
                        float *xBounds = (float*)calloc(nx + 1, sizeof(float));
                        for (i = 0; i <= nx; i++)
                            fscanf(egsphantFile, "%f", xBounds + i);
                        float *yBounds = (float*)calloc(ny + 1, sizeof(float));
                        for (i = 0; i <= ny; i++)
                            fscanf(egsphantFile, "%f", yBounds + i);
                        float *zBounds = (float*)calloc(nz + 1, sizeof(float));
                        for (i = 0; i <= nz; i++)
                            fscanf(egsphantFile, "%f", zBounds + i);
                            
                        int x, y, z;

                        int *mediaIndices = (int*)calloc(nx * ny * nz, sizeof(int));
                        for (z = 0; z < nz; z++) {
                            for (y = 0; y < ny; y++) {
                                for (x = 0; x < nx; x++) {
                                    fscanf(egsphantFile, "%1d", mediaIndices + x + y * nx + z * nx * ny);
                                }
                            }
                            // skip blank line
                            char dummyBuf[1024];
                            fgets(dummyBuf, 1024, egsphantFile);
                        }
                        
                        float *densities = (float*)calloc(nx * ny * nz, sizeof(float));
                        for (z = 0; z < nz; z++) {
                            for (y = 0; y < ny; y++) {
                                for (x = 0; x < nx; x++) {
                                    fscanf(egsphantFile, "%f", densities + x + y * nx + z * nx * ny);
                                }
                            }
                            // skip blank line
                            char dummyBuf[1024];
                            fgets(dummyBuf, 1024, egsphantFile);
                        }
                        
                        // test endianess
                        short int word = 0x0001;
                        char endianess = *(char*)&word; // will be 1 if we're using little-endian or 0 if we're using big endian
                        
                        char iParallel[32];
                        sprintf(iParallel, "%d", i_parallel);
                        
                        // write the density_matrix file
                        string density_matrix_path = egsJoinPath(app_dir, phantomName + ".density_matrix_" + string(iParallel));
                        FILE *density_matrix = fopen(density_matrix_path.c_str(), "wb");
                        intermediate_files_iterator = intermediate_files.insert(intermediate_files_iterator, density_matrix_path);
                        
                        fwrite(&endianess, sizeof(char), 1, density_matrix);
                        fwrite(&nx, sizeof(int), 1, density_matrix);
                        fwrite(&ny, sizeof(int), 1, density_matrix);
                        fwrite(&nz, sizeof(int), 1, density_matrix);
                        fwrite(xBounds, sizeof(float), nx + 1, density_matrix);
                        fwrite(yBounds, sizeof(float), ny + 1, density_matrix);
                        fwrite(zBounds, sizeof(float), nz + 1, density_matrix);
                        fwrite(densities, sizeof(float), nx * ny * nz, density_matrix);
                        
                        fclose(density_matrix);
                        
                        // write the ct_ramp file
                        string ct_ramp_path = egsJoinPath(app_dir, phantomName + ".ct_ramp_" + string(iParallel));
                        FILE *ct_ramp = fopen(ct_ramp_path.c_str(), "wb");
                        intermediate_files_iterator = intermediate_files.insert(intermediate_files_iterator, ct_ramp_path);
                        
                        // the densities that we have already found the medium for
                        vector<float> found_densities;
                        vector<float>::iterator found_it = found_densities.begin();
                        float last_density = 0xFFFFFFFF; // NaN
                        
                        for (i = 0; i < nx * ny * nz; i++) {
                            // found density different than last one
                            if (last_density != densities[i]) {
                                float density = densities[i];
                            
                                // check whether we have previously found this density
                                bool already_found = false;
                                vector<float>::iterator it;
                                for (it = found_densities.begin(); it < found_densities.end(); it++) {
                                    if (*it == density) {
                                        already_found = true;
                                        break;
                                    }
                                }
                                
                                // if this is a new density, add it to the ramp file
                                if (!already_found) {
                                    it = found_densities.insert(it, density);
                                    int mediaIdx = mediaIndices[i];
                                    string med = media[mediaIdx - 1];
                                    int densityI = *(int*)&density;
                                    int minDenI = densityI - 10;
                                    int maxDenI = densityI + 10;
                                    // this is needed to prevent the compiler from optimizing the pointer operations and thus producing undesired results
                                    float *minDen = new float();
                                    float *maxDen = new float();
                                    *minDen = *(float*)&minDenI;
                                    *maxDen = *(float*)&maxDenI;
                                    
                                    fprintf(ct_ramp, "%s %e %e %e\n", med.c_str(), *minDen, *maxDen, density);
                                }
                            }
                        }
                        
                        fclose(ct_ramp);
                        
                        free(xBounds);
                        free(yBounds);
                        free(zBounds);
                        free(mediaIndices);
                        free(densities);

                        // write geometry for phantom
                        fprintf(f, "\n");
                        fprintf(f, ":start geometry:\n");
                        fprintf(f, "    library = egs_ndgeometry\n");
                        fprintf(f, "    type = EGS_XYZGeometry\n");
                        fprintf(f, "    name = %s\n", phantomName.c_str());
                        fprintf(f, "    density matrix = %s\n", density_matrix_path.c_str());
                        fprintf(f, "    ct ramp = %s\n", ct_ramp_path.c_str());
                        fprintf(f, ":stop geometry:\n");
                        fprintf(f, "\n");
                    }
                    else {
                        if (f != NULL)
                            fclose(f);
                        fclose(in);
                        egsFatal("\n\n*** FATAL ERROR ***\nCould not open egsphant file %s\n*******************\n\n", egsphantPath.c_str());
                    }
                }
            }
            else
                fputs(line.c_str(), f);
        }
    }
    else {
        if (f != NULL)
            fclose(f);
        egsFatal("\n\n*** FATAL ERROR ***\nCould not open input file %s\n*******************\n\n", path.c_str());
    }
    
    fclose(in);
    fflush(f);
    
    return intermediate_files;
}

void printHelpMessage() {
    char ESC = 27;

    printf("\n");
    printf("%c[1mUsage:%c[0m  Epp -i INPUT_FILE -p PEGS_FILE [OPTIONS]\n", ESC, ESC);
    printf("\n");
    printf("        %c[1mINPUT_FILE%c[0m is the name of the input file for the simulation;\n", ESC, ESC);
    printf("        the input file must be in the Epp user code directory\n");
    printf("\n");
    printf("        %c[1mPEGS_FILE%c[0m is the path to the PEGS4 file to be used for the simulation;\n", ESC, ESC);
    printf("        the path must either be absolute or relative to $HEN_HOUSE/pegs4/data/\n");
    printf("        or $EGS_HOME/pegs4/data/.\n");
    printf("\n");
    printf("        %c[1mOPTIONS%c[0m are additional command line arguments and switches from the\n", ESC, ESC);
    printf("        list below (optional)\n");
    printf("\n");
    printf("%c[1mRefer to the user manual (Epp User Manual.pdf) for more information.%c[0m\n", ESC, ESC);
    printf("\n");
    printf("The following additional command line arguments can be used to change the\n");
    printf("behavior of the program:\n");
    printf("\n");
    printf("-o OUTPUT_FILE              Sets the names of the output files to %c[1mOUTPUT_FILE%c[0m.\n", ESC, ESC);
    printf("--output OUTPUT_FILE        By default the name of the input file is used.\n");
    printf("\n");
    printf("-pr INPUT_FILE              Epp will parse %c[1mINPUT_FILE%c[0m and resolve all #include\n", ESC, ESC);
    printf("--parse INPUT_FILE          and #egsphant directives and create a single\n");
    printf("                            completed input file. This is useful to check and\n");
    printf("                            debug #include directives and to create an input\n");
    printf("                            file whose geometry can be viewed with egs_view, as\n");
    printf("                            egs_view does not understand #include and #egsphant.\n");
    printf("\n");
    printf("                            This option overrides all other options except -h\n");
    printf("                            and --help.\n");
    printf("\n");
    printf("-oP FLAGS                   Specifies which photon output files will be created.\n");
    printf("--output-photout FLAGS      %c[1mFLAGS%c[0m is any combination of the following letters\n", ESC, ESC);
    printf("                            (any order, without separators):\n");
    printf("                            - %c[1mn%c[0m: none (turn off all files)\n", ESC, ESC);
    printf("                            - %c[1ma%c[0m: all (turn on all files)\n", ESC, ESC);
    printf("                            - %c[1mp%c[0m: primary\n", ESC, ESC);
    printf("                            - %c[1mc%c[0m: Compton\n", ESC, ESC);
    printf("                            - %c[1mm%c[0m: multiple\n", ESC, ESC);
    printf("                            - %c[1mr%c[0m: Rayleigh\n", ESC, ESC);
    printf("                            - %c[1mt%c[0m: total\n", ESC, ESC);
    printf("\n");
    printf("                            Note that %c[1mn%c[0m overrides all other flags and %c[1ma%c[0m\n", ESC, ESC, ESC, ESC);
    printf("                            overrides the individual flags (%c[1mp%c[0m, %c[1mc%c[0m, %c[1mm%c[0m, %c[1mr%c[0m, %c[1mt%c[0m).\n", ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC, ESC);
    printf("\n");
    printf("                            %c[1mExamples%c[0m\n", ESC, ESC);
    printf("                            %c[1mmcr%c[0m:  only multiple, Compton and Rayleigh files will\n", ESC, ESC);
    printf("                                  be created\n");
    printf("                            %c[1mptn%c[0m:  no files will be created because %c[1mn%c[0m overrides\n", ESC, ESC, ESC, ESC);
    printf("                                  all other flags\n");
    printf("                            %c[1man%c[0m:   no files will be created because %c[1mn%c[0m overrides\n", ESC, ESC, ESC, ESC);
    printf("                                  all other flags\n");
    printf("                            %c[1mcart%c[0m: all files will be created because %c[1ma%c[0m overrides\n", ESC, ESC, ESC, ESC);
    printf("                                  all individual flags\n");
    printf("\n");
    printf("                            The default is %c[1mn%c[0m (none).\n", ESC, ESC);
    printf("\n");
    printf("-oC FLAGS                   Specifies which count output files will be created.\n");
    printf("--output-count FLAGS        See above for the description of %c[1mFLAGS%c[0m.\n", ESC, ESC);
    printf("                            The default is %c[1ma%c[0m (all).\n", ESC, ESC);
    printf("\n");
    printf("-oE FLAGS                   Specifies which energy output files will be created.\n");
    printf("--output-energy FLAGS       See above for the description of %c[1mFLAGS%c[0m.\n", ESC, ESC);
    printf("                            The default is %c[1mn%c[0m (none).\n", ESC, ESC);
    printf("\n");
    printf("-oIC FLAGS                  Specifies which image count output files will be\n");
    printf("--output-img-cnt FLAGS      created.\n");
    printf("                            See above for the description of %c[1mFLAGS%c[0m.\n", ESC, ESC);
    printf("                            The default is %c[1ma%c[0m (all).\n", ESC, ESC);
    printf("\n");
    printf("-oIE FLAGS                  Specifies which image energy output files will be\n");
    printf("--output-img-e FLAGS        created.\n");
    printf("                            See above for the description of %c[1mFLAGS%c[0m.\n", ESC, ESC);
    printf("                            The default is %c[1mn%c[0m (none).\n", ESC, ESC);
    printf("\n");
    printf("-oD OPTION                  Specifies that the dose should be calculated in a\n");
    printf("--output-dose OPTION        voxelized geometry specified in the input file.\n");
    printf("                            %c[1mOPTION%c[0m is be one of the following:\n", ESC, ESC);
    printf("                            - %c[1mtext%c[0m: write dose to text file\n", ESC, ESC);
    printf("                            - %c[1mbin%c[0m: write dose to binary file\n", ESC, ESC);
    printf("                            - %c[1mtextbin%c[0m: write dose to text and binary files\n", ESC, ESC);
    printf("\n");
    printf("                            Any other value for %c[1mOPTION%c[0m will result in no dose\n", ESC, ESC);
    printf("                            output being created.\n");
    printf("\n");
    printf("                            The default is no value and thus no dose output.\n");
    printf("\n");
    printf("-e PATH                     Sets the %c[1mEGS_HOME%c[0m path to %c[1mPATH%c[0m instead of the\n", ESC, ESC, ESC, ESC);
    printf("--egs-home PATH             standard path defined by the %c[1mEGS_HOME%c[0m environment\n", ESC, ESC);
    printf("                            variable.\n");
    printf("\n");
    printf("-H PATH                     Sets the %c[1mHEN_HOUSE%c[0m path to %c[1mPATH%c[0m instead of the\n", ESC, ESC, ESC, ESC);
    printf("--hen-house PATH            standard path defined by the %c[1mHEN_HOUSE%c[0m environment\n", ESC, ESC);
    printf("                            variable.\n");
    printf("\n");
    printf("-b                          Runs the simulation in batch mode, i.e. an *.egslog\n");
    printf("--batch                     output file will be created.\n");
    printf("\n");
    printf("-P N                        Specifies the number (%c[1mN%c[0m) of parallel jobs.\n", ESC, ESC);
    printf("--parallel N\n");
    printf("\n");
    printf("-j I                        Specifies the index (%c[1mI%c[0m) of this process in the list\n", ESC, ESC);
    printf("--job I                     of all processes running in parallel.\n");
    printf("\n");
    printf("-s                          Specifies that a simple run control object should be\n");
    printf("--simple-run                used for parallel runs. This option is useful if\n");
    printf("                            there are issues with locking the run control file\n");
    printf("                            otherwise required for parallel runs.\n");
    printf("\n");
    printf("-h                          Prints this message.\n");
    printf("--help\n");
    printf("                            This option overrides all other options.\n");
    printf("\n");
    
    egsFatal("\n");
}

/***** END OF CODE MODIFICATION *****/

class EGS_LOCAL EGS_GeometryHistory {
public:
    struct Step {
        EGS_Vector x, u;
        EGS_Float  twant, t;
        int        ireg, inew;
        Step() {};
        Step(int Ireg, int Inew, const EGS_Vector &X, const EGS_Vector &U, 
            EGS_Float Twant, EGS_Float T) : x(X), u(U), twant(Twant), t(T), 
            ireg(Ireg), inew(Inew) {};
        void show() const {
            egsWarning("old region=%d, position=(%g,%g,%g), "
              "direction=(%g,%g,%g) intended step=%g, new region=%d, "
              "step=%g\n",ireg,x.x,x.y,x.z,u.x,u.y,u.z,twant,inew,t);
        };
    };

    EGS_GeometryHistory(int N=100) : steps(new Step [N]), nsize(N), ns(0), 
                                     wrap(false) {};
    ~EGS_GeometryHistory() { delete [] steps; };
    void  addStep(int ireg, int inew, const EGS_Vector &x,
                  const EGS_Vector &u, EGS_Float twant, EGS_Float t) {
        steps[ns++] = Step(ireg,inew,x,u,twant,t); 
        if( ns >= nsize ) { ns = 0; wrap = true; }
    };
    void reportHistory() const {
        int nhave = wrap ? nsize : ns;
        egsWarning("\n************** Last %d geometry steps:\n",nhave);
        if( wrap ) {
            for(int j=ns; j<nsize; j++) steps[j].show();
        }
        for(int j=0; j<ns; j++) steps[j].show();
    };

    Step  *steps;
    int   nsize;
    int   ns;
    bool  wrap;
};

void EGS_Application::reportGeometryError() {
    ghistory->reportHistory();
}

void EGS_Application::storeGeometryStep(int ireg, int inew, 
       const EGS_Vector &x, const EGS_Vector &u, EGS_Float twant, EGS_Float t){
    ghistory->addStep(ireg,inew,x,u,twant,t);
}

EGS_Application::EGS_Application(int argc, char **argv) : input(0), geometry(0),
    source(0), rndm(0), run(0), last_case(0), current_case(0),
    data_out(0), data_in(0), simple_run(false), a_objects(0), 
    ghistory(new EGS_GeometryHistory) {
    
    app_index = n_apps++;

    if( !active_egs_application ) active_egs_application = this;
    { for(int call=BeforeTransport; call<AfterTransport; call++) 
        ausgab_flag[call] = true;
    }
    { for(int call=AfterTransport; call<UnknownCall; call++) 
        ausgab_flag[call] = false;
    }

    //
    // *** get the name of this application
    //
    if( !getArgument(argc,argv,"-a","--application",app_name) ) app_name = egsStripPath(argv[0]);
    if( !app_name.size() ) egsFatal("%s\n   failed to determine application "
            "name from %s or command line arguments\n",__egs_app_msg1,argv[0]);
    //
    // *** make sure EGS_HOME is set.
    //
    checkEnvironmentVar(argc,argv,"-e","--egs-home","EGS_HOME",egs_home);
    app_dir = egsJoinPath(egs_home,app_name);
    //
    // *** make sure HEN_HOUSE is set.
    //
    checkEnvironmentVar(argc,argv,"-H","--hen-house","HEN_HOUSE",hen_house);
    //
    // *** check if there is an input file specified on the command line
    //

/***** CODE MODIFICATION: ADDED *****/
// Jonas Lippuner, 2009-08-31
// Jonas Lippuner, 2010-04-29 added check for -h and --help switches
// Jonas Lippuner, 2010-04-30 added feature to parse input file

    tempFilesIterator = tempFiles.begin();

    bool print_help = false;
    for(int j=1; j<argc; j++) {
        string tmp = argv[j];
        if( tmp == "-h" || tmp == "--help" ) {
            print_help = true;
            break;
        }
    }
    
    if (print_help) {
        printHelpMessage();
        exit(0);
    }
    
    string parse_file;
    if (getArgument(argc, argv, "-pr", "--parse", parse_file)) {
        if (parse_file.size() > 0) {
            char cwd[1024];
            char *dummyCwd = getcwd(cwd, 1024); // store return value in dummy variable to suppress compiler warning
            if (parse_file.find('/') != 0)
                parse_file = egsJoinPath(string(cwd), parse_file);
        
            string parsedInputPath;
            if (parse_file.find(".egsinp") == parse_file.length() - 7)
                parsedInputPath = parse_file.substr(0, parse_file.length() - 7) + ".complete.egsinp";
            else
                parsedInputPath = parse_file + ".complete.egsinp";
            
            FILE *parsedInputFile = fopen(parsedInputPath.c_str(), "w+");
            PreParseFile(parse_file, getDir(parse_file), parsedInputFile);            
            fclose(parsedInputFile);
            
            printf("Input file was successfully parsed and saved as %s\n", parsedInputPath.c_str());
            exit(0);
        }
    }

/***** END OF CODE MODIFICATION *****/

/***** CODE MODIFICATION: REPLACED *****
Jonas Lippuner, 2010-04-30

    getArgument(argc,argv,"-i","--input",input_file);
        //egsFatal("%s\n  no input file specified\n",__egs_app_msg1);{

 ***** WITH *****/
 
    if (!getArgument(argc,argv,"-i","--input",input_file))
        egsFatal("\n\n*** FATAL ERROR ***\nNo input file specified. Run Epp -h for more information.\n*******************\n\n");

/***** END OF CODE MODIFICATION *****/

    //
    // *** create the name of the working directory 
    //
    char buf[512];
    sprintf(buf,"egsrun_%d_",egsGetPid());
    run_dir = buf;
    if( input_file.size() > 0 ) { run_dir += input_file; run_dir += '_'; }
    else run_dir += "_noinput_";
    run_dir += egsHostName();

    //
    // *** check if there is a PEGS file specified on the command line
    //

/***** CODE MODIFICATION: REPLACED *****
Jonas Lippuner, 2010-04-30

    getArgument(argc,argv,"-p","--pegs-file",pegs_file);
        //gsFatal("%s\n  no PEGS file specified\n",__egs_app_msg1);

 ***** WITH *****/
 
    if (!getArgument(argc,argv,"-p","--pegs-file",pegs_file))
        egsFatal("\n\n*** FATAL ERROR ***\nNo pegs file specified. Run Epp -h for more information.\n*******************\n\n");

/***** END OF CODE MODIFICATION *****/

    //
    // *** check if the pegs file exists.
    //
    if( pegs_file.size() > 0 ) {
        string pdirs = egsJoinPath("pegs4","data");
        vector<string> pegs_paths; pegs_paths.push_back("");
        pegs_paths.push_back(egsJoinPath(egs_home,pdirs));
        pegs_paths.push_back(egsJoinPath(hen_house,pdirs));
        if( !__egs_find_pegsfile(pegs_paths,pegs_file,abs_pegs_file) ) 
            egsFatal("%s\n  the pegs file %s does not exist or is not "
               "readable\n",__egs_app_msg1,pegs_file.c_str());
    }

    //
    // *** check if the input file exists.
    //
    string ifile;
    if( input_file.size() > 0 ) {
        ifile = egsJoinPath(app_dir,input_file);
        if( ifile.find(".egsinp") == string::npos ) ifile += ".egsinp";
        if( EGS_ACCESS(ifile.c_str(),R_OK) ) 
            egsFatal("%s\n  the input file %s does not exist or is not "
                  "readable\n",__egs_app_msg1,ifile.c_str());
    }

    //
    // *** set the output file
    //
    if( !getArgument(argc,argv,"-o","--output",output_file) )
          output_file = input_file;
    if( !output_file.size() ) output_file = "test";
    //
    // *** see if a batch run.
    //
    batch_run = false;
    for(int j=1; j<argc; j++) {
        string tmp = argv[j]; 
        if( tmp == "-b" || tmp == "--batch" ) { 
            batch_run = true;
            //for(int i=j; i<argc-1; i++) argv[i] = argv[i+1];
            //argc--; 
            break;
        }
    }
    //
    // *** see if parallel run.
    //
    string npar, ipar; n_parallel = i_parallel = 0;
    bool have_np = getArgument(argc,argv,"-P","--parallel",npar);
    bool have_ip = getArgument(argc,argv,"-j","--job",ipar);
    if( have_np && have_ip ) {
        n_parallel = ::strtol(npar.c_str(),0,10);
        i_parallel = ::strtol(ipar.c_str(),0,10);
        if( n_parallel < 0 ) {
            egsWarning("%s\n  invalid -P argument %d\n",__egs_app_msg1,
                    n_parallel); n_parallel = 0;
        }
        if( i_parallel < 0 ) {
            egsWarning("%s\n  invalid -j argument %d\n",__egs_app_msg1,
                    i_parallel); i_parallel = 0; n_parallel = 0;
        }
        if( i_parallel > n_parallel ) {
            egsWarning("%s\n  job number (%d) can not be larger than number"
                " of parallel jobs(%d). Turning off parallel option\n",
                __egs_app_msg1,i_parallel,n_parallel);
            n_parallel = 0; i_parallel = 0;
        }
    }
    else if ( have_np || have_ip ) 
        egsWarning("%s\n  to specify a parallel run you need both,"
                " the -P and -j command line options\n",__egs_app_msg1);

    //
    // *** see if user wants simple job control
    //
    {
    for(int j=1; j<argc; j++) {
        string tmp = argv[j];
        if( tmp == "-s" || tmp == "--simple-run" ) {
            simple_run = true;
            //for(int i=j; i<argc-1; i++) argv[i] = argv[i+1];
            //argc--; 
            break;
        }
    }
    }

    final_output_file = output_file;
    if( i_parallel > 0 && n_parallel > 0 ) {
        batch_run = true;
        char buf[1024]; sprintf(buf,"%s_w%d",output_file.c_str(),i_parallel);
        output_file = buf;
    }

    //
    // *** read the input
    //
    input = new EGS_Input; 
    if( ifile.size() > 0 ) {
    
/***** CODE MODIFICATION: REPLACED *****
Jonas Lippuner, 2009-11-20

        if( input->setContentFromFile(ifile.c_str()) ) 
            egsFatal("%s\n  error while reading input file %s\n",
                    __egs_app_msg1,ifile.c_str());

 ***** WITH *****/
    
        char iParallel[32];
        sprintf(iParallel, "%d", i_parallel);
        
        string preParsedInputPath = ifile + ".complete_" + string(iParallel);
        FILE *preParsedInputFile = fopen(preParsedInputPath.c_str(), "w+");
        tempFilesIterator = tempFiles.insert(tempFilesIterator, preParsedInputPath);
        
        vector<string> intermediate_files = PreParseFile(ifile, getDir(ifile), preParsedInputFile);
        tempFiles.insert(tempFilesIterator, intermediate_files.begin(), intermediate_files.end());
        tempFilesIterator = tempFiles.begin();
            
        fclose(preParsedInputFile);
        
        if( input->setContentFromFile(preParsedInputPath.c_str()) ) 
            egsFatal("%s\n  error while reading input file %s\n",
                    __egs_app_msg1,ifile.c_str());
                    
        // create temporary input file containing only the MC transport parameters
        // which are needed by EGS
        // only needs to be done once, hence do it if this is a single run otherwise
        // only the first process does it
        if ((n_parallel == 0) || (i_parallel == 1)) {
            string tempInputfilePath = ifile + ".egsinp";
            tempFilesIterator = tempFiles.insert(tempFilesIterator, tempInputfilePath);
            filebuf tempInputfile;
            tempInputfile.open(tempInputfilePath.c_str(), ios::out);
            ostream tempInputStream(&tempInputfile);
            EGS_Input *transportParameters = input->getInputItem("MC transport parameter");
            
            if (transportParameters == NULL)
                egsFatal("\n\n*** FATAL ERROR ***\nInput file does not contain :MC transport parameter: block.\n*******************\n\n");
            
            transportParameters->print(0, tempInputStream);
            tempInputfile.close();
        }

/***** END OF CODE MODIFICATION *****/

    }

}

bool EGS_Application::getArgument(int &argc, char **argv, 
        const char *name1, const char *name2, string &arg) {
    string n1(name1), n2(name2);
    for(int j=1; j<argc-1; j++) {
        if( n1 == argv[j] || n2 == argv[j] ) {
            arg = argv[j+1]; 
            //for(int i=j; i<argc-2; i++) argv[i] = argv[i+2];
            //argc -= 2; 
            return true;
        }
    }
    return false;
}

string EGS_Application::constructIOFileName(const char *extension, 
        bool with_run_dir) const {
    string iofile = with_run_dir ? egsJoinPath(app_dir,run_dir) : app_dir;
    iofile = egsJoinPath(iofile,output_file);
    if( extension ) iofile += extension;
    return iofile;
}

int EGS_Application::outputData() {
    if( data_out ) delete data_out;
    string ofile = constructIOFileName(".egsdat",true);
    /*
    string ofile = egsJoinPath(app_dir,run_dir);
    ofile = egsJoinPath(ofile,output_file);
    ofile += ".egsdat";
    */
    data_out = new ofstream(ofile.c_str());
    if( !(*data_out) ) {
        egsWarning("EGS_Application::outputData: failed to open %s "
                "for writing\n",ofile.c_str()); return 1;
    }
    if( !run->storeState(*data_out) ) return 2;
    if( !egsStoreI64(*data_out,current_case) ) return 3;
    (*data_out) << endl;
    if( !rndm->storeState(*data_out) ) return 4;
    if( !source->storeState(*data_out) ) return 5;
    return 0;
}

int EGS_Application::readData() {
    if( data_in ) delete data_in;
    string ifile = constructIOFileName(".egsdat",false);
    /*
    string ifile = egsJoinPath(app_dir,output_file);
    ifile += ".egsdat";
    */
    data_in = new ifstream(ifile.c_str());
    if( !(*data_in) ) {
        egsWarning("EGS_Application::readData: failed to open %s "
                "for reading\n",ifile.c_str()); return 1;
    }
    if( !run->setState(*data_in) ) return 2;
    if( !egsGetI64(*data_in,current_case) ) return 3;
    last_case = current_case;
    if( !rndm->setState(*data_in) ) return 4;
    if( !source->setState(*data_in) ) return 5;
    return 0;
}

void EGS_Application::resetCounter() {
    last_case = 0; current_case = 0;
    run->resetCounter(); 
    rndm->resetCounter(); 
    source->resetCounter();
}

int EGS_Application::addState(istream &data) {
    if( !run->addState(data) ) return 1;
    EGS_I64 tmp_case; if( !egsGetI64(data,tmp_case) ) return 2;
    current_case += tmp_case; last_case = current_case;
    if( !rndm->addState(data) ) return 3;
    if( !source->addState(data) ) return 4;
    return 0;
}

int EGS_Application::combineResults() {
    egsInformation(
"\n                      Suming the following .egsdat files:\n"
"=======================================================================\n");
    char buf[512]; resetCounter();
    EGS_Float last_cpu = 0; EGS_I64 last_ncase = 0;
    int ndat = 0; bool ok = true;
    
/***** CODE MODIFICATION: REPLACED *****
Jonas Lippuner, 2009-07-21

    for(int j=1; j<500; j++) {

 ***** WITH *****/
 
    for(int j=1; j<=n_parallel; j++) {

/***** END OF CODE MODIFICATION *****/

        sprintf(buf,"%s_w%d.egsdat",output_file.c_str(),j);
        string dfile = egsJoinPath(app_dir,buf);
        ifstream data(dfile.c_str());
        if( data ) {
            int err = addState(data); ++ndat;
            if( !err ) {
                EGS_I64 ncase = run->getNdone(); 
                EGS_Float cpu = run->getCPUTime();
                egsInformation("%2d %-30s ncase=%-14lld cpu=%-11.2f\n",
                        ndat,buf,ncase-last_ncase,cpu-last_cpu);
                last_ncase = ncase; last_cpu = cpu;
            }
            else {
                ok = false; egsWarning("%2d %-30s error %d\n",ndat,buf,err);
            }
        }
    }
    if( ndat > 0 ) {
        egsInformation(
"=======================================================================\n");
        egsInformation("%40s%-14lld cpu=%-11.2f\n\n","Total ncase=",last_ncase,
                last_cpu);
    }
    if( ndat > 0 ) return ok ? 0 : -1; else return 1;
}

EGS_I64 EGS_Application::randomNumbersUsed() const {
    if( !rndm ) return 0;
    return rndm->numbersUsed();
}

int EGS_Application::initGeometry() {
    if( !input ) return -1;
    //if( geometry ) { delete geometry; geometry = 0; }
    EGS_BaseGeometry::setActiveGeometryList(app_index);
    geometry = EGS_BaseGeometry::createGeometry(input);
    if( !geometry ) return 1;
    geometry->ref();
    return 0;
}

int EGS_Application::initSource() {
    if( !input ) return -1;
    //if( source ) { delete source; source = 0; }
    source = EGS_BaseSource::createSource(input);
    if( !source ) return 1;
    source->ref();
    return 0;
}

int EGS_Application::initRunControl() {
    if( run ) delete run;
    if( simple_run ) run = new EGS_RunControl(this);
    else run = EGS_RunControl::getRunControlObject(this);
    if( !run ) return 1;
    return 0;
}

int EGS_Application::initRNG() {
    if( rndm ) delete rndm;
    int sequence = 0; 
    if( n_parallel > 0 && i_parallel > 0 ) sequence = i_parallel - 1;
    if( input ) rndm = EGS_RandomGenerator::createRNG(input,sequence);
    if( !rndm ) {
        egsWarning("EGS_Application::initRNG(): using default RNG\n");
        rndm = EGS_RandomGenerator::defaultRNG(sequence);
    }
    if( !rndm ) {
        egsWarning("EGS_Application::initRNG(): got null RNG?\n");
        return 1;
    }
    return 0;
}

int EGS_Application::initSimulation() {
    //if( !input ) { egsWarning("%s no input\n",__egs_app_msg2); return -1; }
    egsInformation("In EGS_Application::initSimulation()\n");
    int err; bool ok = true;
    err = initGeometry();
    if( err ) { 
        egsWarning("\n\n%s geometry initialization failed\n",__egs_app_msg2); 
        ok = false;
    }
    err = initSource();
    if( err ) { 
        egsWarning("\n\n%s source initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    err = initRNG();
    if( err ) {
        egsWarning("\n\n%s RNG initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    err = initRunControl();
    if( err ) {
        egsWarning("\n\n%s run control initialization failed\n",__egs_app_msg2);
        ok = false;
    }
    if( !ok ) return 1;
    err = initEGSnrcBackEnd();
    if( err ) {
        egsWarning("\n\n%s back-end initialization failed\n",__egs_app_msg2);
        return 2;
    }
    describeUserCode();
    err = initCrossSections();
    if( err ) {
        egsWarning("\n\n%s cross section initialization failed\n",__egs_app_msg2);
        return 3;
    }
    err = initScoring();
    if( err ) {
        egsWarning("\n\n%s scoring initialization failed with status %d\n",
                __egs_app_msg2,err);
        return 4;
    }
    initAusgabObjects();
    //describeSimulation();
    return 0;
}

void EGS_Application::setSimulationChunk(EGS_I64 nstart, EGS_I64 nrun) {
    if( source ) source->setSimulationChunk(nstart,nrun);
}

void EGS_Application::describeSimulation() {
    if( !geometry && !source ) return;
    egsInformation("\n\n");
    if( geometry ) geometry->printInfo();
    if( source ) egsInformation("\n\nThe simulation uses the following source:"
                                  "\n========================================="
          "\n %s\n\n\n",source->getSourceDescription());
    if( rndm ) { rndm->describeRNG(); egsInformation("\n\n"); }
    if( a_objects_list.size() > 0 ) {
        egsInformation("The following ausgab objects are included in the simulation\n");
        egsInformation("===========================================================\n\n");
        for(int j=0; j<a_objects_list.size(); ++j) 
            egsInformation("%s",a_objects_list[j]->getObjectDescription());
        egsInformation("\n\n");
    }
}

int EGS_Application::runSimulation() {
    bool ok = true;
    if( !geometry ) {
        egsWarning("%s no geometry\n",__egs_app_msg3); ok = false;
    }
    if( !source ) {
        egsWarning("%s no source\n",__egs_app_msg3); ok = false;
    }
    if( !rndm ) {
        egsWarning("%s no RNG\n",__egs_app_msg3); ok = false;
    }
    if( !run ) {
        egsWarning("%s no run control object\n",__egs_app_msg3); ok = false;
    }
    if( !ok ) return 1;

    int start_status = run->startSimulation();
    if( start_status ) {
        if( start_status < 0 ) 
           egsWarning("\n%s failed to start the simulation\n\n",__egs_app_msg3);
        return start_status;
    }

    EGS_I64 ncase; bool next_chunk = true;

    while( next_chunk && (ncase = run->getNextChunk()) > 0 ) {

        egsInformation("\nRunning %lld histories\n",ncase);
        double f,df;
        if( run->getCombinedResult(f,df) ) {
            char c = '%';
            egsInformation("    combined result from this and other parallel"
                  " runs: %lg +/- %7.3lf%c\n\n",f,df,c);
        }
        else egsInformation("\n");
        int nbatch = run->getNbatch();
        EGS_I64 ncase_per_batch = ncase/nbatch;
        if( !ncase_per_batch ) {
            ncase_per_batch = 1; nbatch = ncase;
        }
        for(int ibatch=0; ibatch<nbatch; ibatch++) {
            if( !run->startBatch(ibatch,ncase_per_batch) ) {
                egsInformation("  startBatch() loop termination\n");
                next_chunk = false; break;
            }
            for(EGS_I64 icase=0; icase<ncase_per_batch; icase++) {
                if( simulateSingleShower() ) {
                    egsInformation("  simulateSingleShower() "
                            "loop termination\n");
                    next_chunk = false; break;
                }
            }
            if( !next_chunk ) break;
            if( !run->finishBatch() ) {
                egsInformation("  finishBatch() loop termination\n");
                next_chunk = false; break;
            }
        }
    }
    // call this from within finishSimulation()
    //run->finishSimulation();
    return 0;
}

int EGS_Application::simulateSingleShower() {
    int ireg; int ntry = 0; last_case = current_case;
    do {
        ntry++;
        if( ntry > 100000 ) {
            egsWarning("EGS_Application::simulateSingleShower(): no particle"
                  " from the source has entered the geometry after 100000"
                  " attempts\n"); return 1;
        }
        current_case = 
            source->getNextParticle(rndm,p.q,p.latch,p.E,p.wt,p.x,p.u);
        ireg = geometry->isWhere(p.x);
        if( ireg < 0 ) {
            EGS_Float t = 1e30; ireg = geometry->howfar(ireg,p.x,p.u,t);
            if( ireg >= 0 ) p.x += p.u*t;
        }
    } while ( ireg < 0 );
    p.ir = ireg;
    int err = startNewShower(); if( err ) return err;
    err = shower(); if( err ) return err;
    return finishShower(); 
} 

int EGS_Application::startNewShower() {
    if( current_case != last_case ) {
        for(int j=0; j<a_objects_list.size(); ++j) 
            a_objects_list[j]->setCurrentCase(current_case);
    }
    return 0;
}

EGS_Application::~EGS_Application() {
    if( rndm ) delete rndm;
    if( run ) delete run;
    if( input ) delete input;
    EGS_Object::deleteObject(source);
    if( geometry ) {
        if( !geometry->deref() ) {
            EGS_Application *app = active_egs_application;
            active_egs_application = this;
            EGS_BaseGeometry::setActiveGeometryList(app_index);
            delete geometry;
            active_egs_application = app;
        }
    }
    /*
    if( a_objects ) {
        for(int j=(int)BeforeTransport; j<=(int)AugerEvent; ++j) {
            while( a_objects[j].size() ) EGS_Object::deleteObject(a_objects[j].pop());
        }
        delete [] a_objects;
    }
    */
    if( a_objects ) delete [] a_objects;
    if( ghistory ) delete ghistory;
    if( active_egs_application == this ) active_egs_application = 0;
    
/***** CODE MODIFICATION: ADDED *****/
// Jonas Lippuner, 2009-08-31
    
    // delete temporary input files if they exist
    for (tempFilesIterator = tempFiles.begin(); tempFilesIterator < tempFiles.end(); tempFilesIterator++) {
        FILE *tmpF = fopen(tempFilesIterator->c_str(), "r");
        if (tmpF) {
            fclose(tmpF);
            remove(tempFilesIterator->c_str());
        }
    }

/***** END OF CODE MODIFICATION *****/

}

void EGS_Application::addAusgabObject(EGS_AusgabObject *o) {
    if( !o ) return;
    o->setApplication(this);
    a_objects_list.add(o);
    //int ncall = 1 + (int)AugerEvent;
    int ncall = (int)UnknownCall;
    if( !a_objects ) a_objects = new EGS_SimpleContainer<EGS_AusgabObject *> [ncall];
    for(int call=(int)BeforeTransport; call<ncall; ++call) {
        if( o->needsCall((AusgabCall)call) ) {
            a_objects[call].add(o); setAusgabCall((AusgabCall)call,true);
        }
    }
}

void EGS_Application::initAusgabObjects() {
    if( !input ) return;
    EGS_AusgabObject::createAusgabObjects(input);
    for(int i=0; i<EGS_AusgabObject::nObjects(); ++i) 
        addAusgabObject(EGS_AusgabObject::getObject(i));
}

int EGS_Application::finishSimulation() {
    //I don't think we need a separate analyzeResults function.
    //analyzeResults();
    int err = 0;
    if( run ) {
        err = run->finishSimulation();
        if( err < 0 ) return err;
    }

    for(int j=0; j<a_objects_list.size(); ++j) 
        a_objects_list[j]->reportResults();

    outputResults(); 
    if( data_out ) { 
        delete data_out; data_out = 0; 
        /*
#ifdef WIN32
        string dfile = egsJoinPath(app_dir,run_dir);
        dfile = egsJoinPath(dfile,output_file);
        dfile += ".egsdat";
        string command = "move /Y "; 
        command += dfile; command += "  "; command += app_dir;
        egsInformation("About to execute <%s>\n",command.c_str());
        int istat = system(command.c_str());
        if( istat ) egsWarning("Failed to move the .egsdat file from the"
               " working directory\n");
#endif
        */
    }
    finishRun();
    run_dir = "";  // i.e. from now on all output will go to the user code 
                   // directory.
    return err;
}

void EGS_Application::fillRandomArray(int n, EGS_Float *rarray) {
    rndm->fillArray(n,rarray);
}

void EGS_Application::appInformation(const char *msg) {
    if(msg) { fprintf(stdout,"%s",msg); fflush(stdout); }
}

void EGS_Application::appWarning(const char *msg) {
    if(msg) { fprintf(stderr,"%s",msg); fflush(stderr); }
}

void EGS_Application::appFatal(const char *msg) {
    if(msg) { fprintf(stderr,"%s",msg); fflush(stderr); }
    exit(1);
}
