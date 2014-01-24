//============================================================================
/**
	Implementation file for KeyEvaluationStats.h
	
	@author		Johan Pauwels
	@date		20101008
*/
//============================================================================

// Includes
#include <stdexcept>
#include "KeyEvaluationStats.h"

KeyEvaluationStats::KeyEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix,
									   const size_t inNumOfChromas /*= 12*/)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfChromas(inNumOfChromas), 
  m_NumOfModes((inConfusionMatrix.rows()-1)/inNumOfChromas), 
  m_NumOfKeys(m_NumOfModes * m_NumOfChromas),
  m_KeysMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfKeys, m_NumOfKeys))
{
}

KeyEvaluationStats::~KeyEvaluationStats()
{
	// Nothing to do...
}

const double KeyEvaluationStats::getCorrectKeys() const
{
	return  m_ConfusionMatrix.matrix().diagonal().head(m_NumOfKeys).sum();
}

const double KeyEvaluationStats::getCorrectNoKeys() const
{
	return m_ConfusionMatrix(m_NumOfKeys, m_NumOfKeys);
}

const double KeyEvaluationStats::getKeyDeletions() const
{
	return m_ConfusionMatrix.bottomRows<1>().head(m_NumOfKeys).sum();
}

const double KeyEvaluationStats::getKeyInsertions() const
{
	return m_ConfusionMatrix.rightCols<1>().head(m_NumOfKeys).sum();
}

const double KeyEvaluationStats::getAdjacentKeys() const
{
	return m_KeysMatrix.matrix().diagonal(m_NumOfModes).sum() +
		m_KeysMatrix.matrix().diagonal(-m_NumOfModes).sum() +
		m_KeysMatrix.matrix().diagonal(m_NumOfKeys-m_NumOfModes).sum() +
		m_KeysMatrix.matrix().diagonal(-m_NumOfKeys+m_NumOfModes).sum();
}

const double KeyEvaluationStats::getRelativeKeys() const
{
	if (m_NumOfModes != 2)
	{
		throw std::invalid_argument("Relative keys are only defined for major-minor mode pairs");
	}
	double theRelKeyDuration = 0.;
	Eigen::ArrayXd theMajorSuperDiag = m_KeysMatrix.matrix().diagonal(m_NumOfKeys-3*m_NumOfModes-1);
	Eigen::ArrayXd theMajorSubDiag = m_KeysMatrix.matrix().diagonal(-3*m_NumOfModes-1);
	Eigen::ArrayXd theMinorSuperDiag = m_KeysMatrix.matrix().diagonal(3*m_NumOfModes+1);
	Eigen::ArrayXd theMinorSubDiag = m_KeysMatrix.matrix().diagonal(-m_NumOfKeys+3*m_NumOfModes+1);
	for (Eigen::ArrayXXd::Index i = 0; i < theMajorSuperDiag.size(); ++i)
	{
		if (i % 2 == 1)
		{
			theRelKeyDuration += theMajorSuperDiag[i] + theMinorSubDiag[i];
		}
	}
	for (Eigen::ArrayXXd::Index i = 0; i < theMajorSubDiag.size(); ++i)
	{
		if (i % 2 == 0)
		{
			theRelKeyDuration += theMajorSubDiag[i] + theMinorSuperDiag[i];
		}
	}
	return theRelKeyDuration;
}

const double KeyEvaluationStats::getParallelKeys() const
{
	if (m_NumOfModes != 2)
	{
		throw std::invalid_argument("Parallel keys are only defined for major-minor mode pairs");
	}
	double theParKeyDuration = 0.;
	for (Eigen::ArrayXXd::Index i = 0; i < m_NumOfChromas; ++i)
	{
		theParKeyDuration += m_KeysMatrix(i*m_NumOfModes,i*m_NumOfModes+1) + 
			m_KeysMatrix(i*m_NumOfModes+1,i*m_NumOfModes);
	}
	return theParKeyDuration;
}

const size_t KeyEvaluationStats::getNumOfUniquesInRef() const
{
	return (m_KeysMatrix > 0.).colwise().any().count();
}

const size_t KeyEvaluationStats::getNumOfUniquesInTest() const
{
	return (m_KeysMatrix > 0.).rowwise().any().count();
}
