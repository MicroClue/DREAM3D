/* ============================================================================
 * Copyright (c) 2012 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2012 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "H5FilterParametersReader.h"

#include "MXA/Utilities/StringUtils.h"

#include "H5Support/H5Utilities.h"
#include "H5Support/H5Lite.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersReader::H5FilterParametersReader()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
H5FilterParametersReader::~H5FilterParametersReader()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::openOptionsGroup(AbstractFilter* filter)
{
  int err = 0;
  if (m_GroupId <= 0)
  {
    return -1;
  }

  std::string name = StringUtils::numToString(filter->getPipelineIndex());
  m_CurrentGroupId = H5Gopen(m_GroupId, name.c_str(), H5P_DEFAULT);
  //err = H5Lite::readStringAttribute(m_GroupId, name, "ClassName", filter->getNameOfClass());
  //err = H5Lite::readStringAttribute(m_GroupId, name, "HumanLabel", filter->getHumanLabel());
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::closeOptionsGroup()
{
  H5Gclose(m_CurrentGroupId);
  m_CurrentGroupId = -1;
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, std::string &value)
{
  int err = 0;
  err = H5Lite::readStringDataset(m_CurrentGroupId, name, value);
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, int8_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, int16_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, int32_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, int64_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, uint8_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, uint16_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, uint32_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, uint64_t &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, float &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, double &value)
{
  int err = 0;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, QualityMetricFilter* f)
{
  int err = 0;

  float value = f->getFieldValue();
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  err = H5Lite::readStringAttribute(m_CurrentGroupId, name, "FieldName", f->getFieldName());
  err = H5Lite::readStringAttribute(m_CurrentGroupId, name, "FieldOperator", f->getFieldOperator());
  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, IntVec3Widget_t v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = { 3 };
  err = H5Lite::readPointerDataset<int32_t>(m_CurrentGroupId, name, reinterpret_cast<int32_t*>(&v) );
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, FloatVec3Widget_t v)
{
  int err = 0;
  int32_t rank = 1;
  hsize_t dims[1] = { 3 };
  err = H5Lite::readPointerDataset<float>(m_CurrentGroupId, name, reinterpret_cast<float*>(&v) );
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int H5FilterParametersReader::readValue(const std::string name, ComparisonInput_t v)
{
  int err = 0;

  float value = v.compValue;
  err = H5Lite::readScalarDataset(m_CurrentGroupId, name, value);
  err = H5Lite::readStringAttribute(m_CurrentGroupId, name, "ArrayName", v.arrayName);
  err = H5Lite::readScalarAttribute(m_CurrentGroupId, name, "CompOperator", v.compOperator);
  return err;
}


