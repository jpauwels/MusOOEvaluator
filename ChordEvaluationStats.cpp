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
										   const size_t inNumOfChromas /*= 12*/)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfChromas(inNumOfChromas), 
m_NumOfChordTypes(inConfusionMatrix.cols()/inNumOfChromas),
m_NumOfChords(m_NumOfChordTypes * m_NumOfChromas),
m_OnlyRoots(Eigen::ArrayXd::Zero(inNumOfChromas)), m_OnlyTypes(Eigen::ArrayXd::Zero(m_NumOfChordTypes)),
m_ChordsMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfChords,m_NumOfChords)),
m_HasTestCatchAllChords(inConfusionMatrix.rows() > m_NumOfChords+1),
m_HasRefNoChord(inConfusionMatrix.cols() > m_NumOfChords),
m_CardinalityDiff(m_NumOfChords, m_NumOfChords),
m_NumOfWrongChromas(m_NumOfChords, m_NumOfChords)
{
    for (size_t iRefChord = 0; iRefChord < m_NumOfChords; ++iRefChord)
    {
        for (size_t iTestChord = 0; iTestChord < m_NumOfChords; ++iTestChord)
        {
            m_CardinalityDiff(iTestChord, iRefChord) = inChords[iTestChord].cardinality() - inChords[iRefChord].cardinality();
            m_NumOfWrongChromas(iTestChord, iRefChord) = std::max(inChords[iRefChord].cardinality(), inChords[iTestChord].cardinality()) - inChords[iRefChord].commonChromas(inChords[iTestChord]).size();
        }
    }
	for (int i = 0; i < m_NumOfChromas; ++i)
	{
		const Eigen::Block<const Eigen::ArrayXXd> theRootBlock =
			m_ChordsMatrix.block(i*m_NumOfChordTypes, i*m_NumOfChordTypes, m_NumOfChordTypes, m_NumOfChordTypes);
		m_OnlyRoots[i] = theRootBlock.sum() - theRootBlock.matrix().trace();
        if (m_HasTestCatchAllChords)
        {
            m_OnlyRoots[i] += m_ConfusionMatrix.row(m_NumOfChords+1+i).segment(i*m_NumOfChordTypes, m_NumOfChordTypes).sum();
        }
	}
	for (size_t i = 0; i < m_NumOfChordTypes; ++i)
	{
		for (size_t j = 0; j < m_NumOfChromas; ++j)
		{
			for (size_t k = 0; k < m_NumOfChromas; ++k)
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
    return m_ConfusionMatrix.row(m_NumOfChords).head(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getChordInsertions() const
{
    if (m_HasRefNoChord)
    {
        if (m_HasTestCatchAllChords)
        {
            return m_ConfusionMatrix.rightCols<1>().head(m_NumOfChords).sum() +
                m_ConfusionMatrix.rightCols<1>().tail(m_NumOfChromas).sum();
        }
        else
        {
            return m_ConfusionMatrix.rightCols<1>().head(m_NumOfChords).sum();
        }
    }
    else
    {
        return 0.;
    }
}

const double ChordEvaluationStats::getChordSubstitutions() const
{
    double chordSubstitutions = m_ChordsMatrix.sum() - m_ChordsMatrix.matrix().trace();
    if (m_HasTestCatchAllChords)
    {
        chordSubstitutions += m_ConfusionMatrix.block(m_NumOfChords+1, 0, m_NumOfChromas, m_NumOfChords).sum();
    }
	return chordSubstitutions;
}

const size_t ChordEvaluationStats::getNumOfUniquesInRef() const
{
    if (m_HasTestCatchAllChords)
    {
        return ((m_ChordsMatrix > 0.).colwise().any() ||
            (m_ConfusionMatrix.block(m_NumOfChords+1, 0, m_NumOfChromas, m_NumOfChords) > 0).colwise().any()).count();
    }
    else
    {
        return (m_ChordsMatrix > 0.).colwise().any().count();
    }
}

const size_t ChordEvaluationStats::getNumOfUniquesInTest() const
{
    if (m_HasTestCatchAllChords)
    {
        return (m_ChordsMatrix > 0.).rowwise().any().count() +
            (m_ConfusionMatrix.block(m_NumOfChords+1, 0, m_NumOfChromas, m_NumOfChords) > 0).rowwise().any().count();
    }
    else
    {
        return (m_ChordsMatrix > 0.).rowwise().any().count();
    }
}

const double ChordEvaluationStats::getRefChordsDuration() const
{
	return m_ConfusionMatrix.leftCols(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getRefNoChordsDuration() const
{
    if (m_HasRefNoChord)
    {
        return m_ConfusionMatrix.rightCols<1>().sum();
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
    for (size_t iChordType = 0; iChordType < m_NumOfChordTypes; ++iChordType)
    {
        for (size_t iChroma = 0; iChroma < m_NumOfChromas; ++iChroma)
        {
            outCorrectChordsPerType(iChordType,0) += m_ConfusionMatrix(iChroma*m_NumOfChordTypes+iChordType, iChroma*m_NumOfChordTypes+iChordType);
            outCorrectChordsPerType(iChordType,1) += m_ConfusionMatrix.col(iChroma*m_NumOfChordTypes+iChordType).sum();
        }
    }
    return outCorrectChordsPerType;
}

const double ChordEvaluationStats::getChordsWithNWrong(const size_t inNumOfWrongChromas) const
{
    return (m_NumOfWrongChromas == inNumOfWrongChromas).select(m_ChordsMatrix, 0.).sum();
}

const double ChordEvaluationStats::getChordsWithSDI(const size_t inNumOfSubstitutedChromas, const size_t inNumOfDeletedChromas, const size_t inNumOfInsertedChromas) const
{
    return (m_NumOfWrongChromas == inNumOfSubstitutedChromas && m_CardinalityDiff == -inNumOfDeletedChromas+inNumOfInsertedChromas).select(m_ChordsMatrix, 0.).sum();
}
