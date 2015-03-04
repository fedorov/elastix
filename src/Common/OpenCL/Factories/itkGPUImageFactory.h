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
#ifndef __itkGPUImageFactory_h
#define __itkGPUImageFactory_h

#include "itkGPUObjectFactoryBase.h"
#include "itkGPUImage.h"

namespace itk
{
/** \class GPUImageFactory2
 * \brief Object Factory implementation for GPUImage
 *
 * \author Denis P. Shamonin and Marius Staring. Division of Image Processing,
 * Department of Radiology, Leiden, The Netherlands
 *
 * \note This work was funded by the Netherlands Organisation for
 * Scientific Research (NWO NRG-2010.02 and NWO 639.021.124).
 *
 */
template< typename TTypeList, typename NDimentions >
class GPUImageFactory2 : public GPUObjectFactoryBase< NDimentions >
{
public:

  typedef GPUImageFactory2                    Self;
  typedef GPUObjectFactoryBase< NDimentions > Superclass;
  typedef SmartPointer< Self >                Pointer;
  typedef SmartPointer< const Self >          ConstPointer;

  /** Return a descriptive string describing the factory. */
  const char * GetDescription() const { return "A Factory for GPUImage"; }

  /** Method for class instantiation. */
  itkFactorylessNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( GPUImageFactory2, GPUObjectFactoryBase );

  /** Register one factory of this type. */
  static void RegisterOneFactory();

  /** Operator() to register override. */
  template< typename TType, unsigned int VImageDimension >
  void operator()( void )
  {
    this->RegisterOverride(
      typeid( Image< TType, VImageDimension > ).name(),
      typeid( GPUImage< TType, VImageDimension > ).name(),
      "GPU Image Override", true,
      CreateObjectFunction< GPUImage< TType, VImageDimension > >::New()
      );
  }


protected:

  GPUImageFactory2();
  virtual ~GPUImageFactory2() {}

  /** Register methods for 1D. */
  virtual void Register1D();

  /** Register methods for 2D. */
  virtual void Register2D();

  /** Register methods for 3D. */
  virtual void Register3D();

private:

  GPUImageFactory2( const Self & );  // purposely not implemented
  void operator=( const Self & );    // purposely not implemented

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGPUImageFactory.hxx"
#endif

#endif // end #ifndef __itkGPUImageFactory_h
