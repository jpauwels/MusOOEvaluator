//============================================================================
/**
	Implementation file for ChordEvaluationStats.h
	
	@author		Johan Pauwels
	@date		20100921
*/
//============================================================================

// Includes
#include "ChordEvaluationStats.h"

ChordEvaluationStats::ChordEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix,
                                           const std::vector<MusOO::Chord> inChords,
										   const Eigen::ArrayXXd::Index inNumOfChromas /*= 12*/)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfChromas(inNumOfChromas), 
m_NumOfChordTypes(inConfusionMatrix.rows()/inNumOfChromas),
m_NumOfChords(m_NumOfChordTypes * m_NumOfChromas),
m_OnlyRoots(Eigen::ArrayXd::Zero(inNumOfChromas)), m_OnlyTypes(Eigen::ArrayXd::Zero(m_NumOfChordTypes)),
m_ChordsMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfChords,m_NumOfChords)),
m_HasTestCatchAllChords(inConfusionMatrix.cols() > m_NumOfChords+1),
m_HasRefNoChord(inConfusionMatrix.rows() > m_NumOfChords),
m_CardinalityDiff(m_NumOfChords, m_NumOfChords),
m_NumOfWrongChromas(m_NumOfChords, m_NumOfChords)
{
    for (Eigen::ArrayXXd::Index iTestChord = 0; iTestChord < m_NumOfChords; ++iTestChord)
    {
        for (Eigen::ArrayXXd::Index iRefChord = 0; iRefChord < m_NumOfChords; ++iRefChord)
        {
            m_CardinalityDiff(iRefChord, iTestChord) = inChords[iTestChord].cardinality() - inChords[iRefChord].cardinality();
            m_NumOfWrongChromas(iRefChord, iTestChord) = std::max(inChords[iRefChord].cardinality(), inChords[iTestChord].cardinality()) - inChords[iRefChord].commonChromas(inChords[iTestChord]).size();
        }
    }
	for (int i = 0; i < m_NumOfChromas; ++i)
	{
		const Eigen::Block<const Eigen::ArrayXXd> theRootBlock =
			m_ChordsMatrix.block(i*m_NumOfChordTypes, i*m_NumOfChordTypes, m_NumOfChordTypes, m_NumOfChordTypes);
		m_OnlyRoots[i] = theRootBlock.sum() - theRootBlock.matrix().trace();
        if (m_HasTestCatchAllChords)
        {
            m_OnlyRoots[i] += m_ConfusionMatrix.col(m_NumOfChords+1+i).segment(i*m_NumOfChordTypes, m_NumOfChordTypes).sum();
        }
	}
	for (Eigen::ArrayXXd::Index i = 0; i < m_NumOfChordTypes; ++i)
	{
		for (Eigen::ArrayXXd::Index j = 0; j < m_NumOfChromas; ++j)
		{
			for (Eigen::ArrayXXd::Index k = 0; k < m_NumOfChromas; ++k)
			{
				if (j != k)
				{
					m_OnlyTypes[i] += m_ChordsMatrix(j*m_NumOfChordTypes+i, k*m_NumOfChordTypes+i);
				}
			}
		}
	}
}

ChordEvaluationStats::~ChordEvaluationStats()
{
}

const double ChordEvaluationStats::getCorrectChords() const
{
	return m_ChordsMatrix.matrix().trace();
}

const double ChordEvaluationStats::getCorrectNoChords() const
{
	if (m_HasRefNoChord)
    {
        return m_ConfusionMatrix(m_NumOfChords, m_NumOfChords);
    }
    else
    {
        return 0.;
    }
}

const double ChordEvaluationStats::getChordDeletions() const
{
    return m_ConfusionMatrix.col(m_NumOfChords).head(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getChordInsertions() const
{
    if (m_HasRefNoChord)
    {
        if (m_HasTestCatchAllChords)
        {
            return m_ConfusionMatrix.row(m_NumOfChords).head(m_NumOfChords).sum() +
                m_ConfusionMatrix.row(m_NumOfChords).tail(m_NumOfChromas).sum();
        }
        else
        {
            return m_ConfusionMatrix.row(m_NumOfChords).head(m_NumOfChords).sum();
        }
    }
    else
    {
        return 0.;
    }
}

const double ChordEvaluationStats::getChordSubstitutions() const
{
	return m_ChordsMatrix.sum() - getCorrectChords() + getChordsWithUnknownWrong();
}

const Eigen::ArrayXXd::Index ChordEvaluationStats::getNumOfUniquesInRef() const
{
    if (m_HasTestCatchAllChords)
    {
        return ((m_ChordsMatrix > 0.).rowwise().any() ||
            (m_ConfusionMatrix.block(0, m_NumOfChords+1, m_NumOfChords, m_NumOfChromas) > 0).rowwise().any()).count();
    }
    else
    {
        return (m_ChordsMatrix > 0.).rowwise().any().count();
    }
}

const Eigen::ArrayXXd::Index ChordEvaluationStats::getNumOfUniquesInTest() const
{
    if (m_HasTestCatchAllChords)
    {
        return (m_ChordsMatrix > 0.).colwise().any().count() +
            (m_ConfusionMatrix.block(0, m_NumOfChords+1, m_NumOfChords, m_NumOfChromas) > 0).colwise().any().count();
    }
    else
    {
        return (m_ChordsMatrix > 0.).colwise().any().count();
    }
}

const double ChordEvaluationStats::getRefChordsDuration() const
{
	return m_ConfusionMatrix.topRows(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getRefNoChordsDuration() const
{
    if (m_HasRefNoChord)
    {
        return m_ConfusionMatrix.bottomRows<1>().sum();
    }
    else
    {
        return 0.;
    }
}

const double ChordEvaluationStats::getOnlyRootCorrect() const
{
	return m_OnlyRoots.sum();
}

const double ChordEvaluationStats::getOnlyTypeCorrect() const
{
	return m_OnlyTypes.sum();
}

const double ChordEvaluationStats::getBothRootAndTypeWrong() const
{
	return getChordSubstitutions() - m_OnlyRoots.sum() - m_OnlyTypes.sum();
}

const Eigen::ArrayXXd ChordEvaluationStats::getCorrectChordsPerType() const
{
    Eigen::ArrayXXd outCorrectChordsPerType = Eigen::ArrayXXd::Zero(m_NumOfChordTypes, 2);
    for (Eigen::ArrayXXd::Index iChordType = 0; iChordType < m_NumOfChordTypes; ++iChordType)
    {
        for (Eigen::ArrayXXd::Index iChroma = 0; iChroma < m_NumOfChromas; ++iChroma)
        {
            // Correct chords per type
            outCorrectChordsPerType(iChordType,0) += m_ConfusionMatrix(iChroma*m_NumOfChordTypes+iChordType, iChroma*m_NumOfChordTypes+iChordType);
            // Total duration per type
            outCorrectChordsPerType(iChordType,1) += m_ConfusionMatrix.row(iChroma*m_NumOfChordTypes+iChordType).sum();
        }
    }
    return outCorrectChordsPerType;
}

const double ChordEvaluationStats::getChordsWithNWrong(const Eigen::ArrayXXd::Index inNumOfWrongChromas) const
{
    return (m_NumOfWrongChromas == inNumOfWrongChromas).select(m_ChordsMatrix, 0.).sum();
}

const double ChordEvaluationStats::getChordsWithSDI(const Eigen::ArrayXXd::Index inNumOfSubstitutedChromas, const Eigen::ArrayXXd::Index inNumOfDeletedChromas, const Eigen::ArrayXXd::Index inNumOfInsertedChromas) const
{
    return (m_NumOfWrongChromas == inNumOfSubstitutedChromas && m_CardinalityDiff == inNumOfInsertedChromas-inNumOfDeletedChromas).select(m_ChordsMatrix, 0.).sum();
}

const double ChordEvaluationStats::getChordsWithUnknownWrong() const
{
    if (m_HasTestCatchAllChords)
    {
        return m_ConfusionMatrix.topRightCorner(m_NumOfChords, m_NumOfChromas).sum();
    }
    else
    {
        return 0.;
    }
}
