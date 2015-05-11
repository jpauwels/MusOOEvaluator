#ifndef PairwiseEvaluation_h
#define PairwiseEvaluation_h

//============================================================================
/**
	Template class representing a frame based evaluation of two timed sequences
	of musical properties.

	@author		Johan Pauwels
	@date		20100913
*/
//============================================================================
#include <vector>
#include <set>
#include <iostream>
#include <iterator>
#include <limits>
#include <Eigen/Core>
#include "MusOO/TimedLabel.h"

template<typename T>
class SimilarityScore;

template <typename T>
class PairwiseEvaluation
{
public:

	typedef std::vector<MusOO::TimedLabel<T> > LabelSequence;

	/** Default constructor. */
	PairwiseEvaluation(const std::string& inScoreSelect);

	void evaluate(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inMinRefDuration = 0., const double inMaxRefDuration = std::numeric_limits<double>::infinity(), const double inDelay = 0.);

	const double getOverlapScore() const;
	const Eigen::ArrayXXd& getConfusionMatrix() const;
	const double getTotalDuration() const;
	const std::vector<T>& getLabels() const;
	const size_t getNumOfRefLabels() const;
	const size_t getNumOfTestLabels() const;

	/** Destructor. */
	virtual ~PairwiseEvaluation();

protected:
    void printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const T& theRefLabel, const T& theTestLabel, const T& theMappedRefLabel, const T& theMappedTestLabel, const double theScore, const double theSegmentLength) const;
	
	SimilarityScore<T>* m_Score;
	size_t m_NumOfRefLabels;
    size_t m_NumOfTestLabels;
	Eigen::ArrayXXd m_ConfusionMatrix;
	double m_TotalScore;

private:


};

void printConfusionMatrix(std::ostream& inOutputStream, const Eigen::ArrayXXd& inConfusionMatrix,
						  const std::vector<std::string>& inLabels, const std::string inSeparator = ",",
                          const std::string inQuote = "\"");

template <typename T>
PairwiseEvaluation<T>::~PairwiseEvaluation()
{
	delete m_Score;
}

template <typename T>
void PairwiseEvaluation<T>::evaluate(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inMinRefDuration /*= 0.*/, const double inMaxRefDuration /*= std::numeric_limits<double>::infinity()*/, const double inDelay /*= 0.*/)
{
    m_ConfusionMatrix.setZero();
	m_TotalScore = 0.;
	double theCurTime = inStartTime;
	double thePrevTime;
	double theSegmentLength;
	size_t theRefIndex = 0;
	size_t theTestIndex = 0;
    
	//set end time of test and reference sequence
	double theRefEndTime = 0.;
	double theTestEndTime = 0.;
	if (!inRefSequence.empty())
	{
		theRefEndTime = inRefSequence.back().offset();
	}
	if (!inTestSequence.empty())
	{
		theTestEndTime = inTestSequence.back().offset() - inDelay;
	}
    
	//test whether end time is set
	if (inEndTime <= inStartTime)
	{
		inEndTime = theRefEndTime;
	}
    
	//collect all transition times of both reference and test sequence
    std::set<double> theChangeTimes;
	for (size_t i = 0; i < inRefSequence.size(); i++)
	{
		if (inRefSequence[i].onset() >= inStartTime && inRefSequence[i].onset() < inEndTime)
		{
			theChangeTimes.insert(inRefSequence[i].onset());
		}
		if (inRefSequence[i].offset() >= inStartTime && inRefSequence[i].offset() < inEndTime)
		{
			theChangeTimes.insert(inRefSequence[i].offset());
		}
	}
	for (size_t i = 0; i < inTestSequence.size(); i++)
	{
		if (inTestSequence[i].onset() >= inStartTime && inTestSequence[i].onset() < inEndTime)
		{
			theChangeTimes.insert(inTestSequence[i].onset()-inDelay);
		}
		if (inTestSequence[i].offset() >= inStartTime && inTestSequence[i].offset() < inEndTime)
		{
			theChangeTimes.insert(inTestSequence[i].offset()-inDelay);
		}
	}
	//add test length to change times
	theChangeTimes.insert(inStartTime);
	theChangeTimes.insert(inEndTime);
    
	//run over all the times of chord change
	for (std::set<double>::iterator i = ++theChangeTimes.begin(); i != theChangeTimes.end(); ++i)
	{
		thePrevTime = theCurTime;
		theCurTime = *i;
		theSegmentLength = theCurTime - thePrevTime;
        
		//advance chord segments until in the first segment that ends after the current time
		//or until in the last chord segment
		while (theRefIndex+1 < inRefSequence.size() && inRefSequence[theRefIndex].offset() < theCurTime)
		{
			theRefIndex++;
		}
		while (theTestIndex+1 < inTestSequence.size() && inTestSequence[theTestIndex].offset()-inDelay < theCurTime)
		{
			theTestIndex++;
		}
		/***********************************/
		/* Classification of ended segment */
		/***********************************/
        T theRefLabel;
        double theRefDuration;
		//label in reference
		if (theCurTime <= theRefEndTime && theCurTime > inRefSequence[theRefIndex].onset())
		{
            theRefLabel = inRefSequence[theRefIndex].label();
            theRefDuration = std::min(inRefSequence[theRefIndex].offset(), inEndTime) - std::max(inRefSequence[theRefIndex].onset(), inStartTime);
		}
		//no label in reference
		else
		{
			theRefLabel = T::silence();
            if (theRefIndex > 0 && inRefSequence[theRefIndex-1].offset() >= inStartTime)
            {
                theRefDuration = std::min(inRefSequence[theRefIndex].onset(), inEndTime) - inRefSequence[theRefIndex-1].offset();
            }
            else
            {
                theRefDuration = std::min(inRefSequence[theRefIndex].onset(), inEndTime) - inStartTime;
            }
		}
        T theTestLabel;
        //label in test
        if (theCurTime <= theTestEndTime && theCurTime > inTestSequence[theTestIndex].onset()-inDelay)
        {
            theTestLabel = inTestSequence[theTestIndex].label();
        }
        //no label in test
        else
        {
            theTestLabel = T::silence();
        }
        
        if (theRefDuration >= inMinRefDuration && theRefDuration <= inMaxRefDuration)
        {
            double theScore = m_Score->score(theRefLabel, theTestLabel);
            // NemaEval implementation errors recreation
            //        if (theCurTime > theTestEndTime || theCurTime <= inTestSequence[theTestIndex].onset()-inDelay || theCurTime <= inRefSequence[theRefIndex].onset())
            //        {
            //            theScore = 0.;
            //        }
            if (theScore >= 0)
            {
                m_ConfusionMatrix(m_Score->getRefIndex(), m_Score->getTestIndex()) += theSegmentLength;
                m_TotalScore += theScore * theSegmentLength;
            }
            /******************/
            /* Verbose output */
            /******************/
            if (inVerboseOStream.good())
            {
                printVerboseOutput(inVerboseOStream, thePrevTime, theCurTime, theRefLabel, theTestLabel, m_Score->getMappedRefLabel(), m_Score->getMappedTestLabel(), theScore, theSegmentLength);
            }
        }
	}
}

template <typename T>
const double PairwiseEvaluation<T>::getOverlapScore() const
{
	if (m_ConfusionMatrix.sum() > 0.)
	{
		return m_TotalScore / m_ConfusionMatrix.sum();
	}
	else
	{
		return 0.;
	}
}

template <typename T>
const Eigen::ArrayXXd& PairwiseEvaluation<T>::getConfusionMatrix() const
{
	return m_ConfusionMatrix;
}

template <typename T>
const double PairwiseEvaluation<T>::getTotalDuration() const
{
	return m_ConfusionMatrix.sum();
}

template <typename T>
const std::vector<T>& PairwiseEvaluation<T>::getLabels() const
{
	return m_Score->getLabels();
}

template <typename T>
const size_t PairwiseEvaluation<T>::getNumOfRefLabels() const
{
	return m_NumOfRefLabels;
}

template <typename T>
const size_t PairwiseEvaluation<T>::getNumOfTestLabels() const
{
	return m_NumOfTestLabels;
}

#endif	// #ifndef PairwiseEvaluation_h
