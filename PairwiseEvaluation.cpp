//============================================================================
/**
	Implementation file for PairwiseEvaluation.h
	
	@author		Johan Pauwels
	@date		20100913
*/
//============================================================================

// Includes
#include <iomanip>
#include "MusOO/KeyQM.h"
#include "MusOO/ChordQM.h"
#include "PairwiseEvaluation.h"
#include "SimilarityScoreChord.h"
#include "SimilarityScoreKey.h"
#include "SimilarityScoreNote.h"

using std::set;
using std::endl;
using namespace MusOO;

template <>
PairwiseEvaluation<Key>::PairwiseEvaluation(const std::string& inScoreSelect)
{
	m_SimilarityScore = new SimilarityScoreKey(inScoreSelect);
	m_NumOfRefLabels = m_SimilarityScore->getNumOfRefLabels();
    m_NumOfTestLabels = m_SimilarityScore->getNumOfTestLabels();
}

template <>
PairwiseEvaluation<Chord>::PairwiseEvaluation(const std::string& inScoreSelect)
{
	m_SimilarityScore = new SimilarityScoreChord(inScoreSelect);
	m_NumOfRefLabels = m_SimilarityScore->getNumOfRefLabels();
    m_NumOfTestLabels = m_SimilarityScore->getNumOfTestLabels();
}

template <>
PairwiseEvaluation<Note>::PairwiseEvaluation(const std::string& inScoreSelect)
{
	m_SimilarityScore = new SimilarityScoreNote(inScoreSelect);
	m_NumOfRefLabels = m_SimilarityScore->getNumOfRefLabels();
    m_NumOfTestLabels = m_SimilarityScore->getNumOfTestLabels();
}

template <>
void PairwiseEvaluation<Key>::printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const Key& theRefLabel, const Key& theTestLabel, const Key& theMappedRefLabel, const Key& theMappedTestLabel, const double theScore, const double theSegmentLength) const
{
    inVerboseOStream << theStartTime << "," << theEndTime << "," << KeyQM(theRefLabel) << "," << KeyQM(theTestLabel) << "," << theScore << "," << theSegmentLength << "\n";
}

template <>
void PairwiseEvaluation<Chord>::printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const Chord& theRefLabel, const Chord& theTestLabel, const Chord& theMappedRefLabel, const Chord& theMappedTestLabel, const double theScore, const double theSegmentLength) const
{
    inVerboseOStream << theStartTime << "," << theEndTime << ",\"" << ChordQM(theRefLabel) << "\",\"" << ChordQM(theTestLabel) << "\"," << theScore << "," << theSegmentLength << ",\"" << ChordQM(theMappedRefLabel) << "\",\"" << ChordQM(theMappedTestLabel) << "\",[ ";
    set<Chroma> refChromas = theRefLabel.chromas();
    set<Chroma> testChromas = theTestLabel.chromas();
    for (set<Chroma>::const_iterator it = refChromas.begin(); it != refChromas.end(); ++it)
        inVerboseOStream << it->str() << " ";
    inVerboseOStream << "],[ ";
    for (set<Chroma>::const_iterator it = testChromas.begin(); it != testChromas.end(); ++it)
        inVerboseOStream << it->str() << " ";
    inVerboseOStream << "],";
    inVerboseOStream << theRefLabel.commonChromas(theTestLabel).size();
    inVerboseOStream << ",";
    inVerboseOStream << theRefLabel.bass(true);
    inVerboseOStream << ",";
    inVerboseOStream << theTestLabel.bass(true);
    inVerboseOStream << endl;
//    set<Chroma> common = theRefLabel.commonChromas(theTestLabel);
//    for (set<Chroma>::const_iterator it = common.begin(); it != common.end(); ++it)
//        inVerboseOStream << it->str() << " ";
//    inVerboseOStream << std::endl;
}

template <>
void PairwiseEvaluation<Note>::printVerboseOutput(std::ostream& inVerboseOStream, const double theStartTime, const double theEndTime, const Note& theRefLabel, const Note& theTestLabel, const Note& theMappedRefLabel, const Note& theMappedTestLabel, const double theScore, const double theSegmentLength) const
{
    inVerboseOStream << theStartTime << "," << theEndTime << "," << NoteMidi(theRefLabel) << "," << NoteMidi(theTestLabel) << "," << theScore << "," << theSegmentLength << "\n";
}

void printConfusionMatrix(std::ostream& inOutputStream, const Eigen::ArrayXXd& inConfusionMatrix,
						  const std::vector<std::string>& inLabels, const std::string inSeparator /*= ","*/,
                          const std::string inQuote /*= "\""*/)
{
	inOutputStream << "reference\test" << inSeparator << inQuote;
	copy(inLabels.begin(), inLabels.begin()+inConfusionMatrix.cols()-1, std::ostream_iterator<std::string>(inOutputStream, (inQuote+inSeparator+inQuote).c_str()));
	inOutputStream << inLabels[inConfusionMatrix.cols()-1] << inQuote << endl;
	for (int i = 0; i < inConfusionMatrix.rows(); ++i)
	{
		inOutputStream << inQuote << inLabels[i] << inQuote;
		for (int j = 0; j < inConfusionMatrix.cols(); ++j)
		{
			inOutputStream << inSeparator << std::fixed << std::setprecision(9) << inConfusionMatrix(i,j);
		}
		inOutputStream << endl;
	}
}
