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

	void evaluate(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence, 
                  double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inDelay = 0.);

	const double getOverlapScore() const;
	const Eigen::ArrayXXd& getConfusionMatrix() const;
	const double getTotalDuration() const;
	const std::vector<std::string>& getCategoryLabels() const;
	const size_t getNumOfCategories() const;

	/** Destructor. */
	virtual ~PairwiseEvaluation();

protected:
    void printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const T& theRefLabel, const T& theTestLabel, const T& theMappedRefLabel, const T& theMappedTestLabel, const double theScore, const double theSegmentLength) const;
	
	SimilarityScore<T>* m_Score;
	size_t m_NumOfCategories;
	Eigen::ArrayXXd m_ConfusionMatrix;
	double m_TotalScore;

private:


};


template <typename T>
PairwiseEvaluation<T>::~PairwiseEvaluation()
{
	delete m_Score;
}

template <typename T>
void PairwiseEvaluation<T>::evaluate(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence,
                                     double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inDelay)
{
	m_ConfusionMatrix = Eigen::ArrayXXd::Zero(m_NumOfCategories, m_NumOfCategories);
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
		double theScore = 0.;
        T theRefLabel;
        T theTestLabel;
		//label in reference
		if (theCurTime <= theRefEndTime && theCurTime > inRefSequence[theRefIndex].onset())
		{
            theRefLabel = inRefSequence[theRefIndex].label();
		}
		//no label in reference
		else
		{
			theRefLabel = T::silence();
		}
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
        theScore = m_Score->score(theRefLabel, theTestLabel);
        // NemaEval implementation errors recreation
        //        if (theCurTime > theTestEndTime || theCurTime <= inTestSequence[theTestIndex].onset()-inDelay || theCurTime <= inRefSequence[theRefIndex].onset())
        //        {
        //            theScore = 0.;
        //        }
		if (theScore >= 0)
		{
			m_ConfusionMatrix(m_Score->getTestCategory(),m_Score->getRefCategory()) += theSegmentLength;
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
const std::vector<std::string>& PairwiseEvaluation<T>::getCategoryLabels() const
{
	return m_Score->getCategoryLabels();
}

template <typename T>
const size_t PairwiseEvaluation<T>::getNumOfCategories() const
{
	return m_NumOfCategories;
}

#endif	// #ifndef PairwiseEvaluation_h
