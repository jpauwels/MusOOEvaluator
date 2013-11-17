//============================================================================
/**
	Implementation file for ChordEvaluationStats.h
	
	@author		Johan Pauwels
	@date		20100921
*/
//============================================================================

// Includes
#include "ChordEvaluationStats.h"

ChordEvaluationStats::ChordEvaluationStats(const Eigen::ArrayXXd inConfusionMatrix, 
										   const size_t inNumOfChromas /*= 12*/)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfChromas(inNumOfChromas), 
m_NumOfChordTypes((inConfusionMatrix.rows()-1)/inNumOfChromas), 
m_NumOfChords(m_NumOfChordTypes * m_NumOfChromas),
m_OnlyRoots(Eigen::ArrayXd::Zero(inNumOfChromas)), m_OnlyTypes(Eigen::ArrayXd::Zero(m_NumOfChordTypes)),
m_ChordsMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfChords,m_NumOfChords))
{
	for (int i = 0; i < 12; ++i)
	{
		Eigen::Block<Eigen::ArrayXXd> theRootBlock = 
			m_ConfusionMatrix.block(i*m_NumOfChordTypes,i*m_NumOfChordTypes,
			m_NumOfChordTypes,m_NumOfChordTypes);
		m_OnlyRoots[i] += theRootBlock.sum() - theRootBlock.matrix().diagonal().sum();
	}
	for (size_t i = 0; i < m_NumOfChordTypes; ++i)
	{
		for (size_t j = 0; j < m_NumOfChromas; ++j)
		{
			for (size_t k = 0; k < m_NumOfChromas; ++k)
			{
				if (j != k)
				{
					m_OnlyTypes[i] += m_ConfusionMatrix(j*m_NumOfChordTypes+i, k*m_NumOfChordTypes+i);
				}
			}
		}
	}
	m_CorrectChords = m_ChordsMatrix.matrix().diagonal().sum();
	m_AllWrong = m_ChordsMatrix.sum() - m_CorrectChords - 
		m_OnlyRoots.sum() - m_OnlyTypes.sum();
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
	return m_ConfusionMatrix(m_NumOfChords, m_NumOfChords);
}

const double ChordEvaluationStats::getChordDeletions() const
{
	return m_ConfusionMatrix.bottomRows<1>().head(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getChordInsertions() const
{
	return m_ConfusionMatrix.rightCols<1>().head(m_NumOfChords).sum();
}

const size_t ChordEvaluationStats::getNumOfUniquesInRef() const
{
	return (m_ChordsMatrix > 0.).colwise().any().count();
}

const size_t ChordEvaluationStats::getNumOfUniquesInTest() const
{
	return (m_ChordsMatrix > 0.).rowwise().any().count();
}

const double ChordEvaluationStats::getRefChordsDuration() const
{
	return m_ConfusionMatrix.leftCols(m_NumOfChords).sum();
}

const double ChordEvaluationStats::getRefNoChordsDuration() const
{
	return m_ConfusionMatrix.rightCols<1>().sum();
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
