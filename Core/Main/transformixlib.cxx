/*=========================================================================
 *
 *  Copyright UMC Utrecht and contributors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef __transformixlib_CXX_
#define __transformixlib_CXX_

#include "transformixlib.h"

#ifdef _ELASTIX_USE_MEVISDICOMTIFF
#include "itkUseMevisDicomTiff.h"
#endif

#include "elxTransformixMain.h"
#include <iostream>
#include <string>
#include <vector>
#include <queue>

#include "itkObject.h"
#include "itkDataObject.h"
#include <itksys/SystemTools.hxx>
#include <itksys/SystemInformation.hxx>

#include "itkTimeProbe.h"

namespace transformix
{

/**
 * ******************* Constructor ***********************
 */

TRANSFORMIX::TRANSFORMIX()
{
  this->m_ResultImage = 0;
} // end Constructor


/**
 * ******************* Destructor ***********************
 */

TRANSFORMIX::~TRANSFORMIX()
{
  this->m_ResultImage = 0;
} // end Destructor


/**
 * ******************* GetResultImage ***********************
 */

TRANSFORMIX::ImagePointer
TRANSFORMIX::GetResultImage( void )
{
  return this->m_ResultImage;
} // end GetResultImage()


/**
 * ******************* TransformImage ***********************
 */

int
TRANSFORMIX::TransformImage(
  ImagePointer inputImage,
  std::vector< ParameterMapType > & parameterMaps,
  std::string outputPath,
  bool performLogging,
  bool performCout )
{
  /** Some typedef's.*/
  typedef elx::TransformixMain                        TransformixMainType;
  typedef TransformixMainType::Pointer                TransformixMainPointer;
  typedef TransformixMainType::ArgumentMapType        ArgumentMapType;
  typedef ArgumentMapType::value_type                 ArgumentMapEntryType;
  typedef elx::ElastixMain                            ElastixMainType;
  typedef ElastixMainType::DataObjectContainerType    DataObjectContainerType;
  typedef ElastixMainType::DataObjectContainerPointer DataObjectContainerPointer;

  /** Declare an instance of the Transformix class. */
  TransformixMainPointer transformix;

  DataObjectContainerPointer movingImageContainer = 0;
  DataObjectContainerPointer resultImageContainer = 0;

  /** Initialize. */
  int             returndummy = 0;
  ArgumentMapType argMap;
  bool            outFolderPresent = false;
  std::string     outFolder        = "";
  std::string     logFileName      = "";

  std::string key;
  std::string value;

  if( !outputPath.empty() )
  {
    key   = "-out";
    value = outputPath;

    /** Make sure that last character of the output folder equals a '/'. */
    if( value.find_last_of( "/" ) != value.size() - 1  )
    {
      value.append( "/" );
    }

    outFolderPresent = true;
  }
  else
  {
    /** Put command line parameters into parameterFileList. */
    //there must be an "-out", this is checked later in code!!
    key   = "-out";
    value = "output_path_not_set";
  }

  /** Save this information. */
  outFolder = value;

  /** Attempt to save the arguments in the ArgumentMap. */
  if( argMap.count( key ) == 0 )
  {
    argMap.insert( ArgumentMapEntryType( key.c_str(), value.c_str() ) );
  }
  else if( performCout )
  {
    /** Duplicate arguments. */
    std::cerr << "WARNING!" << std::endl;
    std::cerr << "Argument " << key.c_str() << "is only required once." << std::endl;
    std::cerr << "Arguments " << key.c_str() << " " << value.c_str() << "are ignored" << std::endl;
  }

  if( performLogging )
  {
    /** Check if the output directory exists. */
    bool outFolderExists = itksys::SystemTools::FileIsDirectory( outFolder.c_str() );
    if( !outFolderExists )
    {
      if( performCout )
      {
        std::cerr << "ERROR: the output directory does not exist." << std::endl;
        std::cerr << "You are responsible for creating it." << std::endl;
      }
      return ( -2 );
    }
    else
    {
      /** Setup xout. */
      if( performLogging )
      {
        logFileName = outFolder + "transformix.log";
      }
    }
  }

  /** The argv0 argument, required for finding the component.dll/so's. */
  argMap.insert( ArgumentMapEntryType( "-argv0", "transformix" ) );

  /** Setup xout. */
  int returndummy2 = elx::xoutSetup( logFileName.c_str(), performLogging, performCout );
  if( returndummy2 && performCout )
  {
    if( performCout )
    {
      std::cerr << "ERROR while setting up xout." << std::endl;
    }
    return ( returndummy2 );
  }
  elxout << std::endl;

  /** Declare a timer, start it and print the start time. */
  itk::TimeProbe totaltimer;
  totaltimer.Start();
  elxout << "transformix is started at " << GetCurrentDateAndTime() << ".\n" << std::endl;

  /**
   * ********************* START TRANSFORMATION *******************
   */

  /** Set transformix. */
  transformix = TransformixMainType::New();

  /** Set stuff from input or needed for output */
  movingImageContainer                       = DataObjectContainerType::New();
  movingImageContainer->CreateElementAt( 0 ) = inputImage;
  transformix->SetMovingImageContainer( movingImageContainer );
  transformix->SetResultImageContainer( resultImageContainer );

  /** Run transformix. */
  returndummy = transformix->Run( argMap, parameterMaps );

  /** Check if transformix run without errors. */
  if( returndummy != 0 )
  {
    xl::xout[ "error" ] << "Errors occurred" << std::endl;
    return returndummy;
  }

  /** Get the result image */
  resultImageContainer = transformix->GetResultImageContainer();

  /** Stop timer and print it. */
  totaltimer.Stop();
  elxout << "\nTransformix has finished at " << GetCurrentDateAndTime() << "." << std::endl;
  elxout << "Elapsed time: " << ConvertSecondsToDHMS( totaltimer.GetMean(), 1 ) << ".\n" << std::endl;

  this->m_ResultImage = resultImageContainer->ElementAt( 0 );

  /** Clean up. */
  transformix = 0;
  TransformixMainType::UnloadComponents();

  /** Exit and return the error code. */
  return returndummy;

} // end TransformImage()


/**
 * ******************* TransformImage ***********************
 */

int
TRANSFORMIX::TransformImage(
  ImagePointer inputImage,
  ParameterMapType & parameterMap,
  std::string outputPath,
  bool performLogging,
  bool performCout )
{
  // Transform single parameter map to a one-sized vector of parameter maps and call other
  // transform method.
  std::vector< ParameterMapType > parameterMaps;
  parameterMaps.push_back( parameterMap );
  return TransformImage( inputImage, parameterMaps, outputPath, performLogging, performCout );
} // end TransformImage()


/** ConvertSecondsToDHMS
 *
 */
std::string
TRANSFORMIX::ConvertSecondsToDHMS( const double totalSeconds, const unsigned int precision )
{
  /** Define days, hours, minutes. */
  const std::size_t secondsPerMinute = 60;
  const std::size_t secondsPerHour   = 60 * secondsPerMinute;
  const std::size_t secondsPerDay    = 24 * secondsPerHour;

  /** Convert total seconds. */
  std::size_t       iSeconds = static_cast< std::size_t >( totalSeconds );
  const std::size_t days     = iSeconds / secondsPerDay;

  iSeconds %= secondsPerDay;
  const std::size_t hours = iSeconds / secondsPerHour;

  iSeconds %= secondsPerHour;
  const std::size_t minutes = iSeconds / secondsPerMinute;

  //iSeconds %= secondsPerMinute;
  //const std::size_t seconds = iSeconds;
  const double dSeconds = fmod( totalSeconds, 60.0 );

  /** Create a string in days, hours, minutes and seconds. */
  bool               nonzero = false;
  std::ostringstream make_string( "" );
  if( days    != 0            ) { make_string << days    << "d"; nonzero = true; }
  if( hours   != 0 || nonzero ) { make_string << hours   << "h"; nonzero = true; }
  if( minutes != 0 || nonzero ) { make_string << minutes << "m"; nonzero = true; }
  make_string << std::showpoint << std::fixed << std::setprecision( precision );
  make_string << dSeconds << "s";

  /** Return a value. */
  return make_string.str();

} // end ConvertSecondsToDHMS()


/** Returns current date and time as a string. */
std::string
TRANSFORMIX::GetCurrentDateAndTime( void )
{
  // Obtain current time
  time_t rawtime = time( NULL );
  // Convert to local time
  struct tm * timeinfo = localtime( &rawtime );
  // Convert to human-readable format
  std::string timeAsString = std::string( asctime( timeinfo ) );
  // Erase newline character at end
  timeAsString.erase( timeAsString.end() - 1 );
  //timeAsString.pop_back() // c++11 feature

  return timeAsString;
} // end GetCurrentDateAndTime()


} // namespace transformix

#endif // end #ifndef __transformixlib_CXX_
