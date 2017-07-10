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

#include "MusOO/TimedLabel.h"
#include <Eigen/Core>
#include <vector>
#include <set>
#include <iostream>
#include <iterator>
#include <limits>
#include <numeric>
#include <algorithm>

template<typename T>
class SimilarityScore;

template <typename T>
class PairwiseEvaluation
{
public:

	typedef std::vector<MusOO::TimedLabel<T> > LabelSequence;

	/** Default constructor. */
	PairwiseEvaluation(const std::string& inScoreSelect);
    
    /** Destructor. */
    virtual ~PairwiseEvaluation();

	void addSequencePair(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inMinRefDuration = 0., const double inMaxRefDuration = std::numeric_limits<double>::infinity(), const double inDelay = 0.);

	const std::vector<T>& getLabels() const;
	const Eigen::Index getNumOfRefLabels() const;
	const Eigen::Index getNumOfTestLabels() const;

    // Get results of last file
    const double getDuration() const;
    const double getScore() const;
    const Eigen::ArrayXXd& getConfusionMatrix() const;
    
    // Reductions over data set
    const double calcTotalDuration() const;
    const Eigen::ArrayXXd calcTotalConfusionMatrix() const;
    const double calcAverageScore() const;
    const double calcWeightedAverageScore() const;
    
protected:
    void printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const T& theRefLabel, const T& theTestLabel, const T& theMappedRefLabel, const T& theMappedTestLabel, const double theScore, const double theSegmentLength) const;
	
	SimilarityScore<T>* m_SimilarityScore;
    Eigen::Index m_NumOfRefLabels;
    Eigen::Index m_NumOfTestLabels;
    
    std::vector<double> m_Durations;
    std::vector<double> m_Scores;
    std::vector<Eigen::ArrayXXd> m_ConfusionMatrices;

private:


};

void printConfusionMatrix(std::ostream& inOutputStream, const Eigen::ArrayXXd& inConfusionMatrix,
						  const std::vector<std::string>& inLabels, const std::string inSeparator = ",",
                          const std::string inQuote = "\"");

template <typename T>
PairwiseEvaluation<T>::~PairwiseEvaluation()
{
	delete m_SimilarityScore;
}

template <typename T>
void PairwiseEvaluation<T>::addSequencePair(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inMinRefDuration /*= 0.*/, const double inMaxRefDuration /*= std::numeric_limits<double>::infinity()*/, const double inDelay /*= 0.*/)
{
    m_ConfusionMatrices.push_back(Eigen::ArrayXXd::Zero(m_NumOfRefLabels, m_NumOfTestLabels));
    Eigen::ArrayXXd& curConfusionMatrix = m_ConfusionMatrices.back();
	m_Scores.push_back(0.);
    double& curScore = m_Scores.back();
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
            double theSegmentScore = m_SimilarityScore->score(theRefLabel, theTestLabel);
            // NemaEval implementation errors recreation
            //        if (theCurTime > theTestEndTime || theCurTime <= inTestSequence[theTestIndex].onset()-inDelay || theCurTime <= inRefSequence[theRefIndex].onset())
            //        {
            //            theScore = 0.;
            //        }
            if (theSegmentScore >= 0)
            {
                curConfusionMatrix(m_SimilarityScore->getRefIndex(), m_SimilarityScore->getTestIndex()) += theSegmentLength;
                curScore += theSegmentScore * theSegmentLength;
            }
            /******************/
            /* Verbose output */
            /******************/
            if (inVerboseOStream.good())
            {
                printVerboseOutput(inVerboseOStream, thePrevTime, theCurTime, theRefLabel, theTestLabel, m_SimilarityScore->getMappedRefLabel(), m_SimilarityScore->getMappedTestLabel(), theSegmentScore, theSegmentLength);
            }
        }
	}
    m_Durations.push_back(curConfusionMatrix.sum());
    const double& curDuration = m_Durations.back();
    if (curDuration > 0.)
    {
        curScore /= curDuration;
    }
}

template <typename T>
const std::vector<T>& PairwiseEvaluation<T>::getLabels() const
{
	return m_SimilarityScore->getLabels();
}

template <typename T>
const Eigen::Index PairwiseEvaluation<T>::getNumOfRefLabels() const
{
	return m_NumOfRefLabels;
}

template <typename T>
const Eigen::Index PairwiseEvaluation<T>::getNumOfTestLabels() const
{
	return m_NumOfTestLabels;
}

template <typename T>
const double PairwiseEvaluation<T>::getDuration() const
{
    return m_Durations.back();
}

template <typename T>
const double PairwiseEvaluation<T>::getScore() const
{
    return m_Scores.back();
}

template <typename T>
const Eigen::ArrayXXd& PairwiseEvaluation<T>::getConfusionMatrix() const
{
    return m_ConfusionMatrices.back();
}

template <typename T>
const double PairwiseEvaluation<T>::calcTotalDuration() const
{
    return std::accumulate(m_Durations.begin(), m_Durations.end(), 0.);
}

template <typename T>
const Eigen::ArrayXXd PairwiseEvaluation<T>::calcTotalConfusionMatrix() const
{
    Eigen::ArrayXXd retTotalConfusionMatrix(Eigen::ArrayXXd::Zero(m_NumOfRefLabels, m_NumOfTestLabels));
    for (std::vector<Eigen::ArrayXXd>::const_iterator iConfMat = m_ConfusionMatrices.begin(); iConfMat !=m_ConfusionMatrices.end(); ++iConfMat)
    {
        retTotalConfusionMatrix += *iConfMat;
    }
    return retTotalConfusionMatrix;
}

template <typename T>
const double PairwiseEvaluation<T>::calcAverageScore() const
{
    return std::accumulate(m_Scores.begin(), m_Scores.end(), 0.) / static_cast<double>(m_Scores.size());
}

template <typename T>
const double PairwiseEvaluation<T>::calcWeightedAverageScore() const
{
    return std::inner_product(m_Durations.begin(), m_Durations.end(), m_Scores.begin(), 0.) / calcTotalDuration();
}

#endif	// #ifndef PairwiseEvaluation_h
