//============================================================================
/**
    Implementation file for SegmentationEvaluation.h
 
 @author	Johan Pauwels
 @date		20131009
 */
//============================================================================

// Includes
#include <numeric>
#include <functional>
#include <stdexcept>
#include "SegmentationEvaluation.h"

SegmentationEvaluation::SegmentationEvaluation(const std::string& inVariant, const std::vector<double>& inTolerances)
: m_Variant(inVariant)
, m_Tolerances(inTolerances)
{
}

void SegmentationEvaluation::reset()
{
    m_Durations.clear();
    m_NumRefSegments.clear();
    m_NumTestSegments.clear();
    m_Recalls.clear();
    m_Precisions.clear();
    m_Fmeasures.clear();
    m_MissedBoundaries.clear();
    m_SegmentFragmentations.clear();
    m_CombinedHammingMeasureWorst.clear();
    m_CombinedHammingMeasureHarmonic.clear();
}

void SegmentationEvaluation::addSequencePair(const LabelSequence& inRefSequence, const LabelSequence& inTestSequence)
{
    //    class Onsetor //onset functor
    //    {
    //        void operator()(const MusOO::TimedLabel<std::string>& tl) {tl.onset();}
    //    };
    Eigen::ArrayXd refOnsets(inRefSequence.size());
    std::transform(inRefSequence.begin(), inRefSequence.end(), &refOnsets[0], std::mem_fun_ref<const double&, MusOO::TimedLabel<std::string> >(&MusOO::TimedLabel<std::string>::onset));
    Eigen::ArrayXd refOffsets(inRefSequence.size());
    std::transform(inRefSequence.begin(), inRefSequence.end(), &refOffsets[0], std::mem_fun_ref<const double&, MusOO::TimedLabel<std::string> >(&MusOO::TimedLabel<std::string>::offset));
    Eigen::ArrayXd testOnsets(inTestSequence.size());
    std::transform(inTestSequence.begin(), inTestSequence.end(), &testOnsets[0], std::mem_fun_ref<const double&, MusOO::TimedLabel<std::string> >(&MusOO::TimedLabel<std::string>::onset));
    Eigen::ArrayXd testOffsets(inTestSequence.size());
    std::transform(inTestSequence.begin(), inTestSequence.end(), &testOffsets[0], std::mem_fun_ref<const double&, MusOO::TimedLabel<std::string> >(&MusOO::TimedLabel<std::string>::offset));
    
    m_Durations.push_back(refOffsets[refOffsets.size()-1]);
    
    Eigen::ArrayXd refBoundaries;
    Eigen::ArrayXd testBoundaries;
    if (m_Variant == "Onset")
    {
        refBoundaries = refOnsets;
        testBoundaries = testOnsets;
    }
    else if (m_Variant == "Offset")
    {
        refBoundaries = refOffsets;
        testBoundaries = testOffsets;
    }
    else if (m_Variant == "Inner")
    {
        if (refOnsets.size() > 0)
        {
            refBoundaries = refOnsets.tail(refOnsets.size()-1);
        }
        if (testOnsets.size() > 0)
        {
            testBoundaries = testOnsets.tail(testOnsets.size()-1);
        }
    }
    else if (m_Variant == "Outer")
    {
        if (refOffsets.size() > 0)
        {
            refBoundaries.resize(refOnsets.size()+1);
            refBoundaries << refOnsets, refOffsets.tail<1>();
        }
        else
        {
            refBoundaries = refOnsets;
        }
        if (testOffsets.size() > 0)
        {
            testBoundaries.resize(testOnsets.size()+1);
            testBoundaries << testOnsets, testOffsets.tail<1>();
        }
        else
        {
            testBoundaries = testOnsets;
        }
    }
    else
    {
        throw std::runtime_error("Unknown segmentation variant '" + m_Variant + "'");
    }
    
    m_NumRefSegments.push_back(refOnsets.size());
    m_NumTestSegments.push_back(testOnsets.size());
    
    Eigen::ArrayXd recalls;
    Eigen::ArrayXd precisions;
    segmentationScores(refBoundaries, testBoundaries, recalls, precisions);
    
    m_Recalls.push_back(recalls);
    m_Precisions.push_back(precisions);
    m_Fmeasures.push_back(2 * precisions * recalls / (precisions + recalls));
    m_MissedBoundaries.push_back(missedBoundaries(refOnsets, refOffsets, testOnsets, testOffsets));
    m_SegmentFragmentations.push_back(segmentFragmentation(refOnsets, refOffsets, testOnsets, testOffsets));
    m_CombinedHammingMeasureWorst.push_back(1. - std::max(m_MissedBoundaries.back(), m_SegmentFragmentations.back()));
    m_CombinedHammingMeasureHarmonic.push_back(2./(1./getUnderSegmentation() + 1./getOverSegmentation()));
}

const double SegmentationEvaluation::getDuration() const
{
    return m_Durations.back();
}

const Eigen::ArrayXd::Index SegmentationEvaluation::getNumRefSegments() const
{
    return m_NumRefSegments.back();
}

const Eigen::ArrayXd::Index SegmentationEvaluation::getNumTestSegments() const
{
    return m_NumTestSegments.back();
}

const double SegmentationEvaluation::getUnderSegmentation() const
{
    return 1. - m_MissedBoundaries.back();
}

const double SegmentationEvaluation::getOverSegmentation() const
{
    return 1. - m_SegmentFragmentations.back();
}

const double SegmentationEvaluation::getCombinedHammingMeasureWorst() const
{
    return m_CombinedHammingMeasureWorst.back();
}

const double SegmentationEvaluation::getCombinedHammingMeasureHarmonic() const
{
    return m_CombinedHammingMeasureHarmonic.back();
}

const double SegmentationEvaluation::calcTotalDuration() const
{
    return std::accumulate(m_Durations.begin(), m_Durations.end(), 0.);
}

const double SegmentationEvaluation::calcAverageNumRefSegments() const
{
    return std::accumulate(m_NumRefSegments.begin(), m_NumRefSegments.end(), 0.) / static_cast<double>(m_NumRefSegments.size());
}

const double SegmentationEvaluation::calcAverageNumTestSegments() const
{
    return std::accumulate(m_NumTestSegments.begin(), m_NumTestSegments.end(), 0.) / static_cast<double>(m_NumTestSegments.size());
}

const double SegmentationEvaluation::calcAverageUnderSegmentation() const
{
    return std::accumulate(m_MissedBoundaries.begin(), m_MissedBoundaries.end(), static_cast<double>(m_MissedBoundaries.size()), std::minus<double>()) / m_MissedBoundaries.size();
}

const double SegmentationEvaluation::calcWeightedAverageUnderSegmentation() const
{
    double totalDuration = calcTotalDuration();
    return std::inner_product(m_Durations.begin(), m_Durations.end(), m_MissedBoundaries.begin(), totalDuration, std::minus<double>(), std::multiplies<double>()) / totalDuration;
}

const double SegmentationEvaluation::calcAverageOverSegmentation() const
{
    return std::accumulate(m_SegmentFragmentations.begin(), m_SegmentFragmentations.end(), static_cast<double>(m_SegmentFragmentations.size()), std::minus<double>()) / m_SegmentFragmentations.size();
}

const double SegmentationEvaluation::calcWeightedAverageOverSegmentation() const
{
    double totalDuration = calcTotalDuration();
    return std::inner_product(m_Durations.begin(), m_Durations.end(), m_SegmentFragmentations.begin(), totalDuration, std::minus<double>(), std::multiplies<double>()) / totalDuration;
}

const double SegmentationEvaluation::calcAverageCombinedHammingMeasureWorst() const
{
    return std::accumulate(m_CombinedHammingMeasureWorst.begin(), m_CombinedHammingMeasureWorst.end(), 0.) / m_CombinedHammingMeasureWorst.size();
}

const double SegmentationEvaluation::calcWeightedAverageCombinedHammingMeasureWorst() const
{
    return std::inner_product(m_Durations.begin(), m_Durations.end(), m_CombinedHammingMeasureWorst.begin(), 0.) / calcTotalDuration();
}

const double SegmentationEvaluation::calcAverageCombinedHammingMeasureHarmonic() const
{
    return std::accumulate(m_CombinedHammingMeasureHarmonic.begin(), m_CombinedHammingMeasureHarmonic.end(), 0.) / m_CombinedHammingMeasureHarmonic.size();
}

const double SegmentationEvaluation::calcWeightedAverageCombinedHammingMeasureHarmonic() const
{
    return std::inner_product(m_Durations.begin(), m_Durations.end(), m_CombinedHammingMeasureHarmonic.begin(), 0.) / calcTotalDuration();
}

void SegmentationEvaluation::segmentationScores(const Eigen::ArrayXd& inRefTimeStamps, const Eigen::ArrayXd& inTestTimeStamps, Eigen::ArrayXd& outRecalls, Eigen::ArrayXd& outPrecisions)
{
    outRecalls = Eigen::ArrayXd::Zero(m_Tolerances.size());
    outPrecisions = Eigen::ArrayXd::Zero(m_Tolerances.size());
    if (inRefTimeStamps.size() != 0 && inTestTimeStamps.size() != 0)
    {
        for (Eigen::ArrayXd::Index iRefTime = 0; iRefTime < inRefTimeStamps.size(); ++iRefTime)
        {
            double minDist = (inRefTimeStamps(iRefTime)-inTestTimeStamps).abs().minCoeff();
            for (size_t iTolerance = 0; iTolerance < m_Tolerances.size(); ++iTolerance)
            {
                if (minDist < m_Tolerances[iTolerance])
                {
                    ++outRecalls[iTolerance];
                }
            }
        }
        outRecalls /= inRefTimeStamps.size();
        
        for (Eigen::ArrayXd::Index iTestTime = 0; iTestTime < inTestTimeStamps.size(); ++iTestTime)
        {
            double minDist = (inTestTimeStamps(iTestTime)-inRefTimeStamps).abs().minCoeff();
            for (size_t iTolerance = 0; iTolerance < m_Tolerances.size(); ++iTolerance)
            {
                if (minDist < m_Tolerances[iTolerance])
                {
                    ++outPrecisions[iTolerance];
                }
            }
        }
        outPrecisions /= inTestTimeStamps.size();
    }
}

const double SegmentationEvaluation::missedBoundaries(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets,
                                                       const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets)
{
    if (inTestOnsets.size() != 0)
    {
        // directional Hamming distance / reference track duration
        double dirHammingDist = 0.;
        for (Eigen::ArrayXd::Index iTestLabel = 0; iTestLabel < inTestOnsets.size(); ++iTestLabel)
        {
            Eigen::ArrayXd refOverlap = inRefOffsets.min(inTestOffsets(iTestLabel)) - inRefOnsets.max(inTestOnsets(iTestLabel));
            double maxOverlap = refOverlap.maxCoeff();
            if (maxOverlap > 0)
            {
                dirHammingDist += (refOverlap > 0).select(refOverlap, 0).sum() - maxOverlap;
            }
        }
        return dirHammingDist / inRefOffsets(inRefOffsets.size()-1);
    }
    else
    {
        return 1;
    }
}

const double SegmentationEvaluation::segmentFragmentation(const Eigen::ArrayXd& inRefOnsets, const Eigen::ArrayXd& inRefOffsets,
                                                      const Eigen::ArrayXd& inTestOnsets, const Eigen::ArrayXd& inTestOffsets)
{
    if (inRefOnsets.size() != 0)
    {
        // inverse directional Hamming distance / reference track duration
        double invDirHammingDist = 0.;
        for (Eigen::ArrayXd::Index iRefLabel = 0; iRefLabel < inRefOnsets.size(); ++iRefLabel)
        {
            Eigen::ArrayXd testOverlap = inTestOffsets.min(inRefOffsets(iRefLabel)) - inTestOnsets.max(inRefOnsets(iRefLabel));
            double maxOverlap = testOverlap.maxCoeff();
            if (maxOverlap > 0)
            {
                invDirHammingDist += (testOverlap > 0).select(testOverlap, 0).sum() - maxOverlap;
            }
        }
        return invDirHammingDist / inRefOffsets(inRefOffsets.size()-1);
    }
    else
    {
        return 1;
    }
}

