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
	void evaluate(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence/*, double inStartTime, double inEndTime, std::ostream& inVerboseOStream, const double inDelay = 0.*/);
    const double getUnderSegmentation() const;
    const double getOverSegmentation() const;
    const double getDirectionalHammingMeasure() const;

//	const double getScore() const;
//	const double getTotalDuration() const;
    
protected:
    
    void segmentationScores(const Eigen::ArrayXd& inRefTimeStamps, const Eigen::ArrayXd& inTestTimeStamps, Eigen::ArrayXd& outRecalls, Eigen::ArrayXd& outPrecisions);
    const double underSegmentation(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets, const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets);
    const double overSegmentation(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets, const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets);

    const std::string m_Variant;
    const std::vector<double> m_Tolerances;
//	double m_TotalScore;
//    double m_TotalDuration;
    
    std::vector<Eigen::ArrayXd> m_Recalls;
    std::vector<Eigen::ArrayXd> m_Precisions;
    std::vector<Eigen::ArrayXd> m_Fmeasures;
    std::vector<double> m_UnderSegmentations;
    std::vector<double> m_OverSegmentations;
    std::vector<double> m_DirectionalHammingMeasures;
    
private:
    
    
};

#endif
