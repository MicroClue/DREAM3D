/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2010, Dr. Michael A. Grober (US Air Force Research Laboratories
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "Reconstruction.h"

#include "MXA/MXATypes.h"
#include "MXA/Common/LogTime.h"
#include "MXA/Utilities/MXADir.h"

#include "ANG/AngDirectoryPatterns.h"
#include "ANG/AngReader.h"

#include "DREAM3D/ANGSupport/AbstractAngDataLoader.h"
#include "DREAM3D/ANGSupport/AngDataLoader.h"
#include "DREAM3D/ANGSupport/H5AngDataLoader.h"

#include "DREAM3D/Common/Constants.h"
#include "DREAM3D/DREAM3DConfiguration.h"
#include "DREAM3D/Common/OIMColoring.hpp"
#include "DREAM3D/Reconstruction/ReconstructionVTKWriter.h"
#include "DREAM3D/HDF5/H5ReconStatsWriter.h"
#include "DREAM3D/HDF5/H5GrainWriter.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Reconstruction::Reconstruction() :
m_H5AngFile(""),
m_OutputDirectory("."),
m_OutputFilePrefix("Reconstruction_"),
m_MergeTwins(false),
m_MergeColonies(false),
m_FillinSample(false),
m_MinAllowedGrainSize(0),
m_MinSeedConfidence(0.0),
m_MinSeedImageQuality(0.0),
m_MisorientationTolerance(0.0),
m_Orientation(Ang::NoOrientation),
m_WriteBinaryFiles(true),
m_WriteVisualizationFile(false),
m_WriteIPFFile(false),
m_WriteDisorientationFile(false),
m_WriteImageQualityFile(false),
m_WriteSchmidFactorFile(false),
m_WriteDownSampledFile(false),
m_WriteHDF5GrainFile(false)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Reconstruction::~Reconstruction()
{
}

#define MAKE_OUTPUT_FILE_PATH(outpath, filename)\
    std::string outpath = m_OutputDirectory + MXADir::Separator + m_OutputFilePrefix + filename;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Reconstruction::execute()
{
  //std::cout << "Reconstruction::compute Start" << std::endl;
  int err = -1;

  AbstractAngDataLoader::Pointer oimDataLoader = H5AngDataLoader::New();
  H5AngDataLoader* ptr = dynamic_cast<H5AngDataLoader*>(oimDataLoader.get());
  ptr->setFilename(m_H5AngFile);
  ptr->setZStartIndex(m_ZStartIndex);
  ptr->setZEndIndex(m_ZEndIndex);
  ptr->setOrientation(m_Orientation);

  // Create the ReconstructionFunc object
  m = ReconstructionFunc::New();
  updateProgressAndMessage(("Gathering Size and Resolution Information from OIM Data"), 1);
  err = oimDataLoader->getSizeAndResolution(m->xpoints, m->ypoints, m->zpoints, m->resx, m->resy, m->resz);
  H5AngDataLoader* h5AngLoader = dynamic_cast<H5AngDataLoader*>(oimDataLoader.get());
  if (NULL == h5AngLoader)
  {
    updateProgressAndMessage("ReconstructionFunc Error: Problem casting H5AngDataLoader from super class to sub class", 100);
    return;
  }

  //FIXME: Mike Groeber: Take a look at this setup for the PhaseTypes. I am
  // putting in UnknownTypes just to have something. You can change to suit
  // your needs.

  std::vector<float> precipFractions;
  std::vector<AIM::Reconstruction::CrystalStructure> crystalStructures;
  std::vector<AngPhase::Pointer> phases = h5AngLoader->getPhases();
  crystalStructures.resize(phases.size()+1);

  precipFractions.resize(phases.size() + 1);
  crystalStructures[0] = AIM::Reconstruction::UnknownCrystalStructure;
  m_PhaseTypes[0] = AIM::Reconstruction::UnknownPhaseType;
  precipFractions[0] = -1.0f;
  for(size_t i=0;i<phases.size();i++)
  {
	  int phaseID = phases[i]->getPhase();
	  TSL::OIM::PhaseSymmetry symmetry = phases[i]->getSymmetry();
	  AIM::Reconstruction::CrystalStructure crystal_structure = AIM::Reconstruction::UnknownCrystalStructure;
	  if(symmetry == TSL::OIM::CubicSymmetry) crystal_structure = AIM::Reconstruction::Cubic;
	  else if(symmetry == TSL::OIM::HexagonalSymmetry) crystal_structure = AIM::Reconstruction::Hexagonal;
	  crystalStructures[phaseID] = crystal_structure;

	  precipFractions[phaseID] = -1.0f;
  }

  if (err < 0)
  {
    updateProgressAndMessage("ReconstructionFunc Error: Problem loading data size and resolutions", 100);
     return;
  }
  m->initialize(m->xpoints, m->ypoints, m->zpoints,
                m->resx, m->resy, m->resz, m_MergeTwins, m_MergeColonies, m_MinAllowedGrainSize,
                m_MinSeedConfidence, m_DownSampleFactor, m_MinSeedImageQuality,
                m_MisorientationTolerance, crystalStructures, m_PhaseTypes, precipFractions,
                m_AlignmentMethod);
  m_OutputDirectory = MXADir::toNativeSeparators(m_OutputDirectory);

  // Create a new HDF5 Results file by overwriting any HDF5 file that may be in the way
  MAKE_OUTPUT_FILE_PATH ( hdf5ResultsFile, AIM::Reconstruction::H5StatisticsFile)
  H5ReconStatsWriter::Pointer h5io = H5ReconStatsWriter::New(hdf5ResultsFile);
  if (h5io.get() == NULL)
  {
    updateProgressAndMessage("The HDF5 Statistics file could not be created. Does the path exist and do you have write access to the output directory.", 100);
    m = ReconstructionFunc::NullPointer();  // Clean up the memory
    //std::cout << "Reconstruction::compute Complete" << std::endl;
    return;
  }

  MAKE_OUTPUT_FILE_PATH ( alignmentFile, AIM::Reconstruction::AlignmentFile);
  MAKE_OUTPUT_FILE_PATH ( reconVisFile, AIM::Reconstruction::VisualizationVizFile);
  MAKE_OUTPUT_FILE_PATH ( reconIPFVisFile, AIM::Reconstruction::IPFVizFile);
  MAKE_OUTPUT_FILE_PATH ( reconDisVisFile, AIM::Reconstruction::DisorientationVizFile);
  MAKE_OUTPUT_FILE_PATH ( reconIQVisFile, AIM::Reconstruction::ImageQualityVizFile);
  MAKE_OUTPUT_FILE_PATH ( reconSFVisFile, AIM::Reconstruction::SchmidFactorVizFile);
  MAKE_OUTPUT_FILE_PATH ( reconDSVisFile, AIM::Reconstruction::DownSampledVizFile);
  MAKE_OUTPUT_FILE_PATH ( hdf5GrainFile, AIM::Reconstruction::HDF5GrainFile);

  START_CLOCK()

  updateProgressAndMessage(("Loading Slices"), 4);
  oimDataLoader->loadData(m->voxels.get(), m->xpoints, m->ypoints, m->zpoints);
  m->initializeQuats();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", loadData)

  updateProgressAndMessage(("Finding Border"), 8);
  m->find_border();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", find_border)

  if (m_AlignmentMethod == AIM::Reconstruction::MutualInformation)
  {
      updateProgressAndMessage(("Aligning Slices"), 10);
      m->form_grains_sections();
      CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", form_grains_sections)
  }

  updateProgressAndMessage(("Aligning Slices"), 12);
  m->align_sections(alignmentFile);
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", align_sections)

  updateProgressAndMessage(("Cleaning Data"), 16);
  m->cleanup_data();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", cleanup_data)

  if (m_AlignmentMethod == AIM::Reconstruction::MutualInformation)
  {
      updateProgressAndMessage(("Redefining Border"), 18);
      m->find_border();
      CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", find_border)
  }

  updateProgressAndMessage(("Forming Macro-Grains"), 20);
  m->form_grains();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", form_grains)

  updateProgressAndMessage(("Finding Neighbors"), 24);
  m->find_neighbors();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", find_neighbors)

  updateProgressAndMessage(("Reordering Grains"), 28);
  m->reorder_grains(reconVisFile);
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", reorder_grains)

  if(m_FillinSample == true)
  {
    updateProgressAndMessage(("Creating Smooth Rectangular Sample"), 36);
    m->fillin_sample();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", fillin_sample)
  }

  if (m_MergeTwins == true)
  {
    updateProgressAndMessage(("Merging Twins"), 40);
    m->merge_twins();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", merge_twins)

    updateProgressAndMessage(("Characterizing Twins"), 40);
    m->characterize_twins();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", characterize_twins)

    updateProgressAndMessage(("Renumbering Grains"), 40);
    m->renumber_grains3();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", renumber_grains3)
  }

  if (m_MergeColonies == true)
  {
    updateProgressAndMessage(("Merging Colonies"), 44);
    m->merge_colonies();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", merge_colonies)

    updateProgressAndMessage(("Renumbering Grains"), 44);
    m->characterize_colonies();
    CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", characterize_colonies)
  }


  updateProgressAndMessage(("Finding Grain IPF Colors"), 80);
  m->find_colors();
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", find_colors)

  /** ********** This section writes the VTK files for visualization *** */
  // Create our File Output Writer Object. This will handle all the File Output duties
  ReconstructionVTKWriter::Pointer vtkWriter = ReconstructionVTKWriter::New();
  vtkWriter->setWriteBinaryFiles(m_WriteBinaryFiles);

  updateProgressAndMessage(("Writing VTK Visualization File"), 93);
  if (m_WriteVisualizationFile) {vtkWriter->writeVisualizationFile(m.get(), reconVisFile);}

  updateProgressAndMessage(("Writing VTK Inverse Pole Figure File"), 94);
  if (m_WriteIPFFile) {vtkWriter->writeIPFVizFile(m.get(), reconIPFVisFile);}

  updateProgressAndMessage(("Writing VTK Disorientation File"), 95);
  if (m_WriteDisorientationFile) {vtkWriter->writeDisorientationFile(m.get(), reconDisVisFile);}

  updateProgressAndMessage(("Writing VTK Image Quality File"), 96);
  if (m_WriteImageQualityFile) {vtkWriter->writeImageQualityVizFile(m.get(), reconIQVisFile);}

  updateProgressAndMessage(("Writing VTK Schmid Factor File"), 97);
  if (m_WriteSchmidFactorFile) {vtkWriter->writeSchmidFactorVizFile(m.get(), reconSFVisFile);}

  updateProgressAndMessage(("Writing VTK Down Sampled File"), 98);
  if (m_WriteDownSampledFile) {vtkWriter->writeDownSampledVizFile(m.get(), reconDSVisFile);}
  /** ******* End VTK Visualization File Writing Section ****** */

  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", vtk_viz_files)

  updateProgressAndMessage(("Writing Out HDF5 Grain File. This may take a few minutes to complete."), 99);
  H5GrainWriter::Pointer h5GrainWriter = H5GrainWriter::New();
  if (m_WriteHDF5GrainFile) { h5GrainWriter->writeHDF5GrainsFile(m.get(), hdf5GrainFile); }
  CHECK_FOR_CANCELED(ReconstructionFunc, "Reconstruction was canceled", writeHDF5GrainsFile)


  updateProgressAndMessage(("Reconstruction Complete"), 100);

  // Clean up all the memory by forcibly setting a NULL pointer to the Shared
  // pointer object.
  m = ReconstructionFunc::NullPointer();  // Clean up the memory
  //std::cout << "Reconstruction::compute Complete" << std::endl;

}



#define PRINT_PROPERTY( out, var)\
  out << #var << ": " << m_##var << std::endl;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Reconstruction::printSettings(std::ostream &ostream)
{
  ostream << "Reconstruction Settings Being Used" << std::endl;
    PRINT_PROPERTY(ostream, H5AngFile)
    PRINT_PROPERTY(ostream, ZStartIndex)
    PRINT_PROPERTY(ostream, ZEndIndex)
    PRINT_PROPERTY(ostream, OutputDirectory)
    PRINT_PROPERTY(ostream, MergeTwins)
    PRINT_PROPERTY(ostream, MergeColonies)
    PRINT_PROPERTY(ostream, FillinSample)
    PRINT_PROPERTY(ostream, MinAllowedGrainSize)
    PRINT_PROPERTY(ostream, MinSeedConfidence)
    PRINT_PROPERTY(ostream, DownSampleFactor)
    PRINT_PROPERTY(ostream, MinSeedImageQuality)
    PRINT_PROPERTY(ostream, MisorientationTolerance)
    PRINT_PROPERTY(ostream, AlignmentMethod)
    PRINT_PROPERTY(ostream, Orientation)

    PRINT_PROPERTY(ostream, WriteVisualizationFile)
    PRINT_PROPERTY(ostream, WriteIPFFile)
    PRINT_PROPERTY(ostream, WriteDisorientationFile)
    PRINT_PROPERTY(ostream, WriteImageQualityFile)
    PRINT_PROPERTY(ostream, WriteSchmidFactorFile)
    PRINT_PROPERTY(ostream, WriteDownSampledFile)
    PRINT_PROPERTY(ostream, WriteHDF5GrainFile)
}
