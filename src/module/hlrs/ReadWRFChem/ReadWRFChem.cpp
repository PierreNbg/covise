/******************************************************************
 *
 *    READ WRF CHEM
 *
 *
 *  Description: Read NetCDF-like files from WRF-Chem Simulations
 *  Date: 04.06.19
 *  Author: Leyla Kern
 *
 *******************************************************************/



#include "ReadWRFChem.h"

#include <iostream>
#include <api/coFeedback.h>
#include "do/coDoStructuredGrid.h"
#include "do/coDoData.h"
#include <do/coDoSet.h>

using namespace covise;

const char *NoneChoices[] = { "none" };

std::vector<std::string> varNames;
std::vector<std::string> AxisChoices;

// -----------------------------------------------------------------------------
// constructor
// -----------------------------------------------------------------------------
ReadWRFChem::ReadWRFChem(int argc, char *argv[])
    : coModule(argc, argv, "WRFChem Reader")
{
    ncDataFile = NULL;

    // define parameters

    // File browser
    p_fileBrowser = addFileBrowserParam("NC_file", "NC File");
    p_fileBrowser->setValue("/data/openforecast/testdata_OF.nc", "*.nc");

    // Variables to visualise
    for (int i = 0; i < numParams; i++)
    {
        char namebuf[50];
        sprintf(namebuf, "Variable%d", i);
        p_variables[i] = addChoiceParam(namebuf, namebuf);
        p_variables[i]->setValue(1, NoneChoices, 0);
    }

    // Choice of "coordinate variables" for axes
    p_grid_choice_x = addChoiceParam("GridOutX", "Grid x");
    p_grid_choice_x->setValue(1, NoneChoices, 0);
    p_grid_choice_y = addChoiceParam("GridOutY", "Grid y");
    p_grid_choice_y->setValue(1, NoneChoices, 0);
    p_grid_choice_z = addChoiceParam("GridOutZ", "Grid z");
    p_grid_choice_z->setValue(1, NoneChoices, 0);
    p_date_choice = addChoiceParam("Date","Date or Time");
    p_date_choice->setValue(1, NoneChoices, 0);

    // Vertical scale box
    p_verticalScale = addFloatParam("VerticalScale", "VerticalScale");
    p_verticalScale->setValue(1.0);

    // define ports

    // 3D grid
    p_grid_out = addOutputPort("outPort", "StructuredGrid", "Grid output");
    // 3D uniform grid
    p_unigrid_out = addOutputPort("uniGridPort", "StructuredGrid", "Uniform grid output");
    // 2D Surface
    p_surface_out = addOutputPort("surfaceOut", "Polygons", "2D Grid output");

    // Data ports
    for (int i = 0; i < numParams; i++)
    {
        char namebuf[50];
        sprintf(namebuf, "dataOutPort%d", i);
        p_data_outs[i] = addOutputPort(namebuf, "Float", namebuf);
    }
}

// -----------------------------------------------------------------------------
// destructor
// -----------------------------------------------------------------------------
ReadWRFChem::~ReadWRFChem()
{
    if (ncDataFile)
    {
        delete ncDataFile;
    }
}

// -----------------------------------------------------------------------------
// change of parameters callback
// -----------------------------------------------------------------------------
void ReadWRFChem::param(const char *paramName, bool inMapLoading)
{
    //sendInfo("param callback");

    if (openNcFile())
    {
        if (ncDataFile->getVarCount() < 4) //check that there are at least 3 grid and one data variable
        {
            sendError("file does not contain enough variables (required > 3)");
            for (int i = 0; i < numParams; i++)
            {
                p_variables[i]->disable();
            }
            p_grid_choice_x->disable();
            p_grid_choice_y->disable();
            p_grid_choice_z->disable();
            p_verticalScale->disable();
            p_date_choice->disable();
            return;
        }
        // enable parameter menus for selection
        p_grid_choice_x->enable();
        p_grid_choice_y->enable();
        p_grid_choice_z->enable();
        p_verticalScale->enable();
        p_date_choice->enable();
        for (int i = 0; i < numParams; i++)
        {
            p_variables[i]->enable();
        }
        // Create "Variable" menu entries for all 2D and 3D variables
        NcVar *var;
        int num2d3dVars = 0;
        char *VarDisplayList[100];

        std::multimap<std::string, NcVar> allVars = ncDataFile->getVars();
        for (const auto& var : allVars)
        {
            if (var.second.getDimCount() >= 0)
            {
                varNames.push_back(var.first);

                // A list of info to display for each variable
                char *dispListEntry = new char[50];
                if (var.second.getDimCount() > 0)
                {
                    sprintf(dispListEntry, "%s (%dD) : [", var.first.c_str(), var.second.getDimCount());
                    if (var.second.getDimCount() > 1)
                    {
                        for (int j = 0; j < var.second.getDimCount() - 1; j++)
                        {
                            sprintf(dispListEntry, "%s %s,", dispListEntry, var.second.getDim(j).getName().c_str());
                        }
                        sprintf(dispListEntry, "%s %s ]", dispListEntry, var.second.getDim(var.second.getDimCount() - 1).getName().c_str());
                    }else
                    {
                        sprintf(dispListEntry, "%s %s ]", dispListEntry, var.second.getDim(0).getName().c_str());
                    }
                }else
                {
                    sprintf(dispListEntry, "%s (%dD)", var.first.c_str(), var.second.getDimCount());
                }
                VarDisplayList[num2d3dVars] = dispListEntry;
                num2d3dVars++;
            }
        }

        // Fill the menu. Default selection = last selected. (#1 initially).
        for (int i = 0; i < numParams; i++)
        {
            p_variables[i]->setValue(num2d3dVars, VarDisplayList, p_variables[i]->getValue());
        }
       // Create "Axis" menu entries for 2D variables only
        int num2dVars = 0;
        for (const auto& var : allVars)
        {
            if (var.second.getDimCount() > 0)//(var->num_dims() == 2 || var->num_dims() == 1)
            {
                char* newEntry = new char[var.first.length()];
                strcpy(newEntry, var.first.c_str());
                AxisChoices[num2dVars] = newEntry;
                num2dVars++;
            }
        }

        // Fill axis menus
        p_grid_choice_x->setValue(num2dVars,
                                  AxisChoices, p_grid_choice_x->getValue());
        p_grid_choice_y->setValue(num2dVars,
                                  AxisChoices, p_grid_choice_y->getValue());
        p_grid_choice_z->setValue(num2dVars,
                                  AxisChoices, p_grid_choice_z->getValue());

        p_date_choice->setValue(num2dVars, AxisChoices, p_date_choice->getValue());

    }
    else
    {
        // No nc file, disable parameters
        for (int i = 0; i < numParams; i++)
        {
            p_variables[i]->disable();
        }
        p_grid_choice_x->disable();
        p_grid_choice_y->disable();
        p_grid_choice_z->disable();
        p_verticalScale->disable();
        p_date_choice->disable();
    }
}

// -----------------------------------------------------------------------------
// compute callback
// -----------------------------------------------------------------------------
int ReadWRFChem::compute(const char *)
{
    char buf[128];
    sendInfo("compute call");
    has_timesteps = 0;

    if (ncDataFile->getVarCount() > 0)
    {
        // get variable name from choice parameters
        NcVar var;
        std::vector<NcDim> dims;
        int numdims = 0;
        // Find the variable with most dimensions and record its size
        for (int i = 0; i < numParams; i++)
        {
            var = ncDataFile->getVar(varNames[p_variables[i]->getValue()]);
            if (var.getDimCount() > numdims)
            {
                numdims = var.getDimCount();
                dims = var.getDims();
            }
        }

        NcVar varDate;
       /* char *dateVal[edges[0]];
        for (int i = 0; i < varDate->num_dims(); ++i)
        {
            dateVal[i]  = varDate->as_string(i);
        }*/
        if (dims[0].getSize() > 1)
        {
            has_timesteps = 1;
            varDate = ncDataFile->getVar(AxisChoices[p_date_choice->getValue()]);
            sendInfo("Found %ld time steps\n", (long)dims[0].getSize());
        }


        int nx = 1, ny = dims[numdims - 2].getSize(), nz = dims[numdims - 1].getSize(), nTime = dims[0].getSize();
        if (has_timesteps > 0)
        {
            if (numdims > 3)
            {
                nx = dims[numdims - 3].getSize();
            }
        }else if (numdims >=3){
            nx = dims[numdims - 3].getSize();
        }

        // 1.) GRID
        coDistributedObject **time_grid;
        coDistributedObject **time_unigrid;
        float *x_coord, *y_coord, *z_coord;

        // Find the variable corresponding to the users choice for each axis
        NcVar varX = ncDataFile->getVar(
            AxisChoices[p_grid_choice_x->getValue()]);
        NcVar varY = ncDataFile->getVar(
            AxisChoices[p_grid_choice_y->getValue()]);
        NcVar varZ = ncDataFile->getVar(
            AxisChoices[p_grid_choice_z->getValue()]);

        // Read the grid point values from the file
        float* xVals = new float[varX.getDim(0).getSize()];
        float* yVals = new float[varY.getDim(0).getSize()];
        float* zVals = new float[varZ.getDim(0).getSize()];
        varX.getVar(xVals);
        varY.getVar(yVals);
        varZ.getVar(zVals);

        float scale = p_verticalScale->getValue();
        int m, n = 0;

        //Time dependency
        if (has_timesteps > 0)
        {
            time_grid = new coDistributedObject *[nTime + 1];
            time_grid[nTime] = NULL;

            for (int t = 0; t < nTime; ++t) //TODO: is there a better way to split grid by timestep???
            {
                sprintf(buf, "%s_%d",p_grid_out->getObjName(), t);
                coDoStructuredGrid *outGrid = new coDoStructuredGrid(buf, nx, ny, nz);
                outGrid->getAddresses(&x_coord, &y_coord, &z_coord);
                time_grid[t] = outGrid;
                int delta = t*(nx*ny*nz);

                float scale = p_verticalScale->getValue();
                int m, n = 0;
                for (int i = 0; i < nx; i++) //99
                {
                    m = 0;
                    for (int j = 0; j < ny; j++) //500
                        for (int k = 0; k < nz; k++, m++, n++) //500
                        {
                            x_coord[n] = xVals[m+delta];
                            y_coord[n] = yVals[m+delta];
                            z_coord[n] = zVals[m+delta]*scale;//zVals[i] * scale;
                        }
                }


            }
            coDoSet *time_outGrid = new coDoSet(p_grid_out->getObjName(), time_grid);
            sprintf(buf, "1 %d", nTime);
            time_outGrid->addAttribute("TIMESTEP", buf);
            p_grid_out->setCurrentObject(time_outGrid);

            delete [] time_grid;

        }else {

            coDoStructuredGrid *outGrid = new coDoStructuredGrid(
                p_grid_out->getObjName(), nx, ny, nz);

            // Get the addresses of the 3 coord arrays (each nz*ny*nx long)
            outGrid->getAddresses(&x_coord, &y_coord, &z_coord);


            for (int i = 0; i < nx; i++) //99
            {
                m = 0;
                for (int j = 0; j < ny; j++) //500
                    for (int k = 0; k < nz; k++, m++, n++) //500
                    {
                        x_coord[n] = xVals[m];
                        y_coord[n] = yVals[m];
                        z_coord[n] = zVals[m]*scale;//zVals[i] * scale;
                    }
            }
       }


       // 1. b) UNIFORM MESH
        float *x_unicoord, *y_unicoord, *z_unicoord;
        if (has_timesteps > 0)
        {
            time_unigrid = new coDistributedObject *[nTime + 1];
            time_unigrid[nTime] = NULL;

            for (int t = 0; t < nTime; ++t) //TODO: is there a better way to split grid by timestep???
            {
                sprintf(buf, "%s_%d",p_unigrid_out->getObjName(), t);
                coDoStructuredGrid *outUniGrid = new coDoStructuredGrid(buf, nx, ny, nz);
                outUniGrid->getAddresses(&x_unicoord, &y_unicoord, &z_unicoord);
                time_unigrid[t] = outUniGrid;
                int delta = t*(nx*ny*nz);

                int m, n = 0;
                for (int i = 0; i < nx; i++)
                {
                    m = 0;
                    for (int j = 0; j < ny; j++)
                        for (int k = 0; k < nz; k++, m++, n++)
                        {
                            x_unicoord[n] = i;
                            y_unicoord[n] = j;
                            z_unicoord[n] = k;
                        }
                }

            }
            coDoSet *time_outUniGrid = new coDoSet(p_grid_out->getObjName(), time_unigrid);
            sprintf(buf, "1 %d", nTime);
            time_outUniGrid->addAttribute("TIMESTEP", buf);
            p_unigrid_out->setCurrentObject(time_outUniGrid);

            delete [] time_unigrid;

        }else
        {

            coDoStructuredGrid *outUniGrid = new coDoStructuredGrid(
                p_unigrid_out->getObjName(), nx, ny, nz);

            // Get the addresses of the 3 coord arrays (each nz*ny*nx long)
            outUniGrid->getAddresses(&x_unicoord, &y_unicoord, &z_unicoord);

            n = 0;

            for (int i = 0; i < nx; i++) //99
            {
                m = 0;
                for (int j = 0; j < ny; j++) //500
                    for (int k = 0; k < nz; k++, m++, n++) //500
                    {
                        x_unicoord[n] = i;  //k
                        y_unicoord[n] = j;  //j
                        z_unicoord[n] = k;  //i
                    }
            }
        }

        // 2.) SURFACE
        coDistributedObject **time_surf;
        int numPolygons = (ny - 1) * (nz - 1);
        int *vl, *pl;

        if (has_timesteps > 0)
        {
            time_surf = new coDistributedObject *[nTime + 1];
            time_surf[nTime] = NULL;

            for(int t = 0; t < nTime; ++t)
            {
                sprintf(buf, "%s_%d",p_surface_out->getObjName(), t);
                coDoPolygons *outsurf = new coDoPolygons(buf, ny * nz, numPolygons * 4, numPolygons);
                outsurf->getAddresses(&x_coord, &y_coord, &z_coord, &vl, &pl);
                time_surf[t] = outsurf;
                int delta = t*(nx*ny*nz);
                n = 0;
                for (int j = 0; j < ny; j++)
                    for (int k = 0; k < nz; k++, n++)
                    {
                        x_coord[n] = xVals[n+delta];
                        y_coord[n] = yVals[n+delta];
                        z_coord[n] = 0; //zVals[0];
                    }

                // Fill the vertex list
                n = 0;
                for (int j = 1; j < ny; j++)
                    for (int k = 1; k < nz; k++)
                    {
                        vl[n++] = (j - 1) * nz + (k - 1) + delta;
                        vl[n++] = j * nz + (k - 1) + delta;
                        vl[n++] = j * nz + k + delta;
                        vl[n++] = (j - 1) * nz + k + delta;
                    }
                // Fill the polygon list
                for (int p = 0; p < numPolygons; p++)
                    pl[p] = p * 4 + delta;

            }
            coDoSet *time_outSurf = new coDoSet(p_surface_out->getObjName(),time_surf);
            sprintf(buf, "1 %d", nTime);
            time_outSurf->addAttribute("TIMESTEP", buf);
            p_surface_out->setCurrentObject(time_outSurf);

            delete []  time_surf;

        }else {

            coDoPolygons *outSurface = new coDoPolygons(p_surface_out->getObjName(),
                                                    ny * nz, numPolygons * 4, numPolygons);

            outSurface->getAddresses(&x_coord, &y_coord, &z_coord, &vl, &pl);

            n = 0;
            for (int j = 0; j < ny; j++)
                for (int k = 0; k < nz; k++, n++)
                {
                    x_coord[n] = xVals[n];
                    y_coord[n] = yVals[n];
                    z_coord[n] = 0; //zVals[0];
                }

            // Fill the vertex list
            n = 0;
            for (int j = 1; j < ny; j++)
                for (int k = 1; k < nz; k++)
                {
                    vl[n++] = (j - 1) * nz + (k - 1);
                    vl[n++] = j * nz + (k - 1);
                    vl[n++] = j * nz + k;
                    vl[n++] = (j - 1) * nz + k;
                }
            // Fill the polygon list
            for (int p = 0; p < numPolygons; p++)
                pl[p] = p * 4;
        }
        // Delete buffers from grid replication
        delete[] xVals;
        delete[] yVals;
        delete[] zVals;

        // 3.) DATA
        coDistributedObject ** time_data;
        if (has_timesteps > 0)
        {
            time_data = new coDistributedObject *[nTime + 1];
            time_data[nTime] = NULL;
            for (int i = 0; i < numParams; ++i)
            {
                var = ncDataFile->getVar(varNames[p_variables[i]->getValue()]);
                for(int t = 0; t < nTime; ++t)
                {
                    float *floatData;
                    sprintf(buf, "%s_%d",p_data_outs[i]->getObjName(), t);
                    coDoFloat *outdata = new coDoFloat(buf, nx*ny*nz);
                    outdata->getAddress(&floatData);
                    time_data[t] = outdata;
                    std::vector<size_t> start{ (size_t)t,0,0,0 };
                    std::vector<size_t> size{ 1,(size_t)nx,(size_t)ny,(size_t)nz };
                    int delta = t*(nx*ny*nz);
                    var.getVar(start, size, floatData);
                }

                coDoSet *time_outData = new coDoSet(p_data_outs[i]->getObjName(),time_data);
                sprintf(buf, "1 %d", nTime);
                time_outData->addAttribute("TIMESTEP", buf);
                p_data_outs[i]->setCurrentObject(time_outData);


            }
            delete [] time_data;

        }else {

            coDoFloat *dataOut[numParams];
            float *floatData;
            // FIXME: Assuming all data to be floats
            // For each output variable, create an output object and fill it with data
            for (int i = 0; i < numParams; i++)
            {
                var = ncDataFile->getVar(varNames[p_variables[i]->getValue()]);
                dataOut[i] = new coDoFloat(p_data_outs[i]->getObjName(), nx * ny * nz);
                dataOut[i]->getAddress(&floatData);
                // FIXME: should make sure only to read a 3D subset
                var.getVar( floatData);
            }
        }

        // interaction info for COVER
        coFeedback feedback("ReadWRFChemPlugin");
        feedback.addPara(p_fileBrowser);

        feedback.addPara(p_grid_choice_x);
        feedback.addPara(p_grid_choice_y);
        feedback.addPara(p_grid_choice_z);

        for (int i = 0; i < numParams; ++i)
        {
            feedback.addPara(p_variables[i]);
        }

        // FIXME: check for data first
        //feedback.apply(outGrid);
       // feedback.apply(outSurface);
    }

    return CONTINUE_PIPELINE;
}

// -----------------------------------------------------------------------------
// open the nc file
// -----------------------------------------------------------------------------
bool ReadWRFChem::openNcFile()
{
    string sFileName = p_fileBrowser->getValue();

    if (sFileName.empty())
    {
        sendInfo("WRFChem filename is empty!");
        return false;
    }
    else
    {
		try {
			ncDataFile = new NcFile(sFileName.c_str(), NcFile::read, NcFile::classic64);
		}
		catch (...)
		{
			sendInfo("Couldn't open WRFChem file!");
			return false;
		}
		return true;
    }
}

// -----------------------------------------------------------------------------
// main: create and start the module
// -----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    covise::coModule *application = new ReadWRFChem(argc, argv);
    application->start(argc, argv);

    return 0;
}
