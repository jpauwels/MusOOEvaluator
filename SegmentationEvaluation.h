#ifndef SegmentationEvaluation_h
#define SegmentationEvaluation_h

//============================================================================
/**
     Template class calculating the segmentation scores.
 
 @author	Johan Pauwels
 @date		20131008
 */
//============================================================================
#include <vector>
#include <Eigen/Core>
#include "MusOO/TimedLabel.h"

class SegmentationEvaluation
{
public:
    
	typedef std::vector<MusOO::TimedLabel<std::string> > LabelSequence;
    
	/** Default constructor. */
	SegmentationEvaluation(const std::string& inVariant, const std::vector<double>& inTolerances);
    
    void reset();
	void addSequencePair(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence/*, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inDelay = 0.*/);
    
    // Get results of last file
    const double getDuration() const;
    const Eigen::ArrayXd::Index getNumRefSegments() const;
    const Eigen::ArrayXd::Index getNumTestSegments() const;
    const double getUnderSegmentation() const;
    const double getOverSegmentation() const;
    const double getCombinedHammingMeasureWorst() const;
    const double getCombinedHammingMeasureHarmonic() const;
    
    // Reductions over data set
    const double calcTotalDuration() const;
    const double calcAverageNumRefSegments() const;
    const double calcAverageNumTestSegments() const;
    const double calcAverageSegmentsRatio() const;
    const double calcAverageUnderSegmentation() const;
    const double calcWeightedAverageUnderSegmentation() const;
    const double calcAverageOverSegmentation() const;
    const double calcWeightedAverageOverSegmentation() const;
    const double calcAverageCombinedHammingMeasureWorst() const;
    const double calcWeightedAverageCombinedHammingMeasureWorst() const;
    const double calcAverageCombinedHammingMeasureHarmonic() const;
    const double calcWeightedAverageCombinedHammingMeasureHarmonic() const;

    
protected:
    
    void segmentationScores(const Eigen::ArrayXd& inRefTimeStamps, const Eigen::ArrayXd& inTestTimeStamps, Eigen::ArrayXd& outRecalls, Eigen::ArrayXd& outPrecisions);
    const double missedBoundaries(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets, const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets);
    const double segmentFragmentation(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets, const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets);

    const std::string m_Variant;
    const std::vector<double> m_Tolerances;
    
    std::vector<double> m_Durations;
    std::vector<Eigen::ArrayXd::Index> m_NumRefSegments;
    std::vector<Eigen::ArrayXd::Index> m_NumTestSegments;
    std::vector<Eigen::ArrayXd> m_Recalls;
    std::vector<Eigen::ArrayXd> m_Precisions;
    std::vector<Eigen::ArrayXd> m_Fmeasures;
    std::vector<double> m_MissedBoundaries;
    std::vector<double> m_SegmentFragmentations;
    std::vector<double> m_CombinedHammingMeasureWorst;
    std::vector<double> m_CombinedHammingMeasureHarmonic;
    
private:
    
    
};

#endif
