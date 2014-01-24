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
										   const size_t inNumOfChromas /*= 12*/)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfChromas(inNumOfChromas), 
m_NumOfChordTypes(inConfusionMatrix.cols()/inNumOfChromas),
m_NumOfChords(m_NumOfChordTypes * m_NumOfChromas),
m_OnlyRoots(Eigen::ArrayXd::Zero(inNumOfChromas)), m_OnlyTypes(Eigen::ArrayXd::Zero(m_NumOfChordTypes)),
m_ChordsMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfChords,m_NumOfChords)),
m_HasTestCatchAllChords(inConfusionMatrix.rows() > m_NumOfChords+1),
m_HasRefNoChord(inConfusionMatrix.cols() > m_NumOfChords)
{
	for (int i = 0; i < m_NumOfChromas; ++i)
	{
		const Eigen::Block<const Eigen::ArrayXXd> theRootBlock =
			m_ChordsMatrix.block(i*m_NumOfChordTypes,i*m_NumOfChordTypes,
			m_NumOfChordTypes,m_NumOfChordTypes);
		m_OnlyRoots[i] = theRootBlock.sum() - theRootBlock.matrix().trace();
        if (m_HasTestCatchAllChords)
        {
            m_OnlyRoots[i] += m_ConfusionMatrix.block(m_NumOfChords+1+i, i*m_NumOfChordTypes, 1, m_NumOfChordTypes).sum();
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
	m_CorrectChords = m_ChordsMatrix.matrix().trace();
	m_AllWrong = m_ChordsMatrix.sum() - m_CorrectChords - m_OnlyRoots.sum() - m_OnlyTypes.sum();
    if (m_HasTestCatchAllChords)
    {
        m_AllWrong += m_ConfusionMatrix.block(m_NumOfChords+1, 0, m_NumOfChromas, m_NumOfChords).sum();
    }
}

ChordEvaluationStats::~ChordEvaluationStats()
{
	// Nothing to do...
}

const double ChordEvaluationStats::getCorrectChords() const
{
	return m_CorrectChords;
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

const double ChordEvaluationStats::getAllWrong() const
{
	return m_AllWrong;
}
