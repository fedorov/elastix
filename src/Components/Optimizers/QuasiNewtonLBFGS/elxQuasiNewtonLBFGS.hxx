#ifndef __elxQuasiNewtonLBFGS_hxx
#define __elxQuasiNewtonLBFGS_hxx

#include "elxQuasiNewtonLBFGS.h"
#include <iomanip>
#include <string>
#include "vnl/vnl_math.h"

namespace elastix
{
using namespace itk;


	/**
	 * ********************* Constructor ****************************
	 */
	
	template <class TElastix>
		QuasiNewtonLBFGS<TElastix>
		::QuasiNewtonLBFGS() 
	{
		this->m_LineOptimizer = LineOptimizerType::New();
    this->SetLineSearchOptimizer( this->m_LineOptimizer );
		this->m_EventPasser = EventPassThroughType::New();
		this->m_EventPasser->SetCallbackFunction( this, &Self::InvokeIterationEvent );
    this->m_LineOptimizer->AddObserver( IterationEvent(), this->m_EventPasser );
		this->m_LineOptimizer->AddObserver( StartEvent(), this->m_EventPasser );

		this->m_SearchDirectionMagnitude = 0.0;
		this->m_StartLineSearch = false;	

	} // end Constructor
	
	
	/**
	 * ***************** InvokeIterationEvent ************************
	 */
	
	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>::
		InvokeIterationEvent(const EventObject & event)
	{
		if( typeid( event ) == typeid( StartEvent ) )
		{
			this->m_StartLineSearch = true;
		}
		else
		{
			this->m_StartLineSearch = false;
		}
		
		this->InvokeEvent( IterationEvent() );

		this->m_StartLineSearch = false;
	} // end InvokeIterationEvent


	/**
	 * ***************** StartOptimization ************************
	 */
	
	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>::
		StartOptimization(void)
	{

		/** Check if the entered scales are correct and != [ 1 1 1 ...] */

		this->SetUseScales(false);
		const ScalesType & scales = this->GetScales();
		if ( scales.GetSize() == this->GetInitialPosition().GetSize() )
		{
      ScalesType unit_scales( scales.GetSize() );
			unit_scales.Fill(1.0);
			if (scales != unit_scales)
			{
				/** only then: */
				this->SetUseScales(true);
			}
		}

		this->Superclass1::StartOptimization();

	} //end StartOptimization


	/**
	 * ***************** DeterminePhase *****************************
	 * 
	 * This method gives only sensible output if it is called
	 * during iterating
	 */

	template <class TElastix>
		const char * QuasiNewtonLBFGS<TElastix>::
    DeterminePhase(void) const
	{
		
		if ( this->GetInLineSearch() )
		{
			return "LineOptimizing";
		}

		return "Main";

	} // end DeterminePhase


	/**
	 * ***************** BeforeRegistration ***********************
	 */

	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>::
		BeforeRegistration(void)
	{

		
		/** Add target cells to xout["iteration"].*/
		xout["iteration"].AddTargetCell("1a:SrchDirNr");
		xout["iteration"].AddTargetCell("1b:LineItNr");
		xout["iteration"].AddTargetCell("2:Metric");
		xout["iteration"].AddTargetCell("3:StepLength");
		xout["iteration"].AddTargetCell("4a:||Gradient||");
		xout["iteration"].AddTargetCell("4b:||SearchDir||");
		xout["iteration"].AddTargetCell("4c:DirGradient");
		xout["iteration"].AddTargetCell("5:Phase");
    xout["iteration"].AddTargetCell("6a:Wolfe1");
		xout["iteration"].AddTargetCell("6b:Wolfe2");
	
		/** Format the metric and stepsize as floats */			
		xl::xout["iteration"]["2:Metric"]		<< std::showpoint << std::fixed;
		xl::xout["iteration"]["3:StepLength"] << std::showpoint << std::fixed;
		xl::xout["iteration"]["4a:||Gradient||"] << std::showpoint << std::fixed;
		xl::xout["iteration"]["4b:||SearchDir||"] << std::showpoint << std::fixed;
		xl::xout["iteration"]["4c:DirGradient"] << std::showpoint << std::fixed;
		
		/** \todo: call the correct functions */

	} // end BeforeRegistration


	/**
	 * ***************** BeforeEachResolution ***********************
	 */

	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>
		::BeforeEachResolution(void)
	{
		/** Get the current resolution level.*/
		unsigned int level = static_cast<unsigned int>(
			this->m_Registration->GetAsITKBaseType()->GetCurrentLevel() );
				
		/** Set the maximumNumberOfIterations.*/
		unsigned int maximumNumberOfIterations = 100;
		this->m_Configuration->ReadParameter( maximumNumberOfIterations , "MaximumNumberOfIterations", level );
		this->SetMaximumNumberOfIterations( maximumNumberOfIterations );
		
		/** Set the maximumNumberOfIterations used for a line search.*/
		unsigned int maximumNumberOfLineSearchIterations = 20;
		this->m_Configuration->ReadParameter( maximumNumberOfLineSearchIterations , "MaximumNumberOfLineSearchIterations", level );
		this->m_LineOptimizer->SetMaximumNumberOfIterations( maximumNumberOfLineSearchIterations );
		
		/** Set the length of the initial step, used to bracket the minimum. */
		double stepLength = 1.0; 
		this->m_Configuration->ReadParameter( stepLength,
			"StepLength", level );
		this->m_LineOptimizer->SetInitialStepLengthEstimate(stepLength);
    
		/** Set the LineSearchValueTolerance */
		double lineSearchValueTolerance = 0.0001;
		this->m_Configuration->ReadParameter( lineSearchValueTolerance,
			"LineSearchValueTolerance", level );
		this->m_LineOptimizer->SetValueTolerance(lineSearchValueTolerance);

		/** Set the LineSearchGradientTolerance */
		double lineSearchGradientTolerance = 0.9;
		this->m_Configuration->ReadParameter( lineSearchGradientTolerance,
			"LineSearchGradientTolerance", level );
		this->m_LineOptimizer->SetGradientTolerance(lineSearchGradientTolerance);

		/** Set the GradientMagnitudeTolerance */
		double gradientMagnitudeTolerance = 0.000001;
		this->m_Configuration->ReadParameter( gradientMagnitudeTolerance,
			"GradientMagnitudeTolerance", level );
		this->SetGradientMagnitudeTolerance(gradientMagnitudeTolerance);

		/** Set the Memory */
		unsigned int LBFGSUpdateAccuracy = 5;
		this->m_Configuration->ReadParameter( LBFGSUpdateAccuracy,
			"LBFGSUpdateAccuracy", level );
		this->SetMemory(LBFGSUpdateAccuracy);

		this->m_SearchDirectionMagnitude = 0.0;
		this->m_StartLineSearch = false;
				
	} // end BeforeEachResolution


	/**
	 * ***************** AfterEachIteration *************************
	 */

	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>
		::AfterEachIteration(void)
	{
		
		/** Print some information. */
		
		if ( this->GetStartLineSearch() )
		{
			xl::xout["iteration"]["1b:LineItNr"] << "start";
			m_SearchDirectionMagnitude =
				this->m_LineOptimizer->GetLineSearchDirection().magnitude();
		}
		else
		{
			/** 
			 * If we are in a line search iteration the current line search
			 * iteration number is printed.
			 * If we are in a "main" iteration (no line search) the last
			 * line search iteration number (so the number of line search
			 * iterations minus one) is printed out.
			 */
   	  xl::xout["iteration"]["1b:LineItNr"] <<
	  		this->m_LineOptimizer->GetCurrentIteration();
		}

		if ( this->GetInLineSearch() )
		{
			xl::xout["iteration"]["2:Metric"]	<<
				this->m_LineOptimizer->GetCurrentValue();
			xl::xout["iteration"]["3:StepLength"] <<
				this->m_LineOptimizer->GetCurrentStepLength();
			LineOptimizerType::DerivativeType cd;
			this->m_LineOptimizer->GetCurrentDerivative(cd);
			xl::xout["iteration"]["4a:||Gradient||"] << cd.magnitude();
		} // end if in line search
		else
		{
			xl::xout["iteration"]["2:Metric"]	<<
				this->GetCurrentValue();
			xl::xout["iteration"]["3:StepLength"] << 
				this->GetCurrentStepLength(); 
			xl::xout["iteration"]["4a:||Gradient||"] << 
		    this->GetCurrentGradient().magnitude();
		} // end else (not in line search)
	
		xl::xout["iteration"]["1a:SrchDirNr"]		<< this->GetCurrentIteration();
		xl::xout["iteration"]["5:Phase"]    << this->DeterminePhase();
		xl::xout["iteration"]["4b:||SearchDir||"] << 
			this->m_SearchDirectionMagnitude ;
		xl::xout["iteration"]["4c:DirGradient"] <<
				this->m_LineOptimizer->GetCurrentDirectionalDerivative();
		if ( this->m_LineOptimizer->GetSufficientDecreaseConditionSatisfied() )
		{
		  xl::xout["iteration"]["6a:Wolfe1"] << "true";
		}
		else
		{
			xl::xout["iteration"]["6a:Wolfe1"] << "false";
		}
		if ( this->m_LineOptimizer->GetCurvatureConditionSatisfied() )
		{
		  xl::xout["iteration"]["6b:Wolfe2"] << "true";
		}
		else
		{
			xl::xout["iteration"]["6b:Wolfe2"] << "false";
		}

		if ( !(this->GetInLineSearch()) )
		{
			/** If new samples: compute a new gradient and value. These
			 * will be used in the computation of a new search direction */
			if ( this->GetNewSamplesEveryIteration() )
			{
				this->SelectNewSamples();
        try
				{
					this->GetScaledValueAndDerivative(
					  this->GetScaledCurrentPosition(), 
					  this->m_CurrentValue,
					  this->m_CurrentGradient );
				}
				catch ( ExceptionObject& err )
				{
					this->m_StopCondition = MetricError;
					this->StopOptimization();
					throw err;
				}
			} //end if new samples every iteration
		} // end if not in line search
		
	} // end AfterEachIteration


	/**
	 * ***************** AfterEachResolution *************************
	 */

	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>
		::AfterEachResolution(void)
	{
		/**
    typedef enum {
      MetricError,
      LineSearchError,
      MaximumNumberOfIterations,
      InvalidDiagonalMatrix,
      GradientMagnitudeTolerance,
      Unknown } 
			*/
		
		std::string stopcondition;
		
		switch( this->GetStopCondition() )
		{
	
			case MetricError :
			  stopcondition = "Error in metric";	
			  break;	

			case LineSearchError :
				stopcondition = "Error in LineSearch";
    		break;
	  
			case MaximumNumberOfIterations :
			  stopcondition = "Maximum number of iterations has been reached";	
			  break;	
	
			case InvalidDiagonalMatrix :
			  stopcondition = "The diagonal matrix is invalid";	
			  break;	

			case GradientMagnitudeTolerance :
			  stopcondition = "The gradient magnitude has (nearly) vanished";	
			  break;	
					
		  default:
			  stopcondition = "Unknown";
			  break;
		}

		/** Print the stopping condition */
		elxout << "Stopping condition: " << stopcondition << "." << std::endl;

	} // end AfterEachResolution
	
	/**
	 * ******************* AfterRegistration ************************
	 */

	template <class TElastix>
		void QuasiNewtonLBFGS<TElastix>
		::AfterRegistration(void)
	{
	  /** Print the best metric value */
		
		double bestValue = this->GetCurrentValue();
		elxout
			<< std::endl
			<< "Final metric value  = " 
			<< bestValue
			<< std::endl;
		
	} // end AfterRegistration




} // end namespace elastix

#endif // end #ifndef __elxQuasiNewtonLBFGS_hxx


