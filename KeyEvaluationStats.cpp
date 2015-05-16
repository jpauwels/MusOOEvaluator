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
									   const Eigen::ArrayXXd::Index inNumOfChromas /*= 12*/)
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
	return  m_KeysMatrix.matrix().trace();
}

const double KeyEvaluationStats::getCorrectNoKeys() const
{
	return m_ConfusionMatrix(m_NumOfKeys, m_NumOfKeys);
}

const double KeyEvaluationStats::getKeyDeletions() const
{
	return m_ConfusionMatrix.rightCols<1>().head(m_NumOfKeys).sum();
}

const double KeyEvaluationStats::getKeyInsertions() const
{
	return m_ConfusionMatrix.bottomRows<1>().head(m_NumOfKeys).sum();
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
	Eigen::ArrayXd theRefMajorSuperDiag = m_KeysMatrix.matrix().diagonal(3*m_NumOfModes+1);
    Eigen::ArrayXd theRefMajorSubDiag = m_KeysMatrix.matrix().diagonal(-m_NumOfKeys+3*m_NumOfModes+1);
    Eigen::ArrayXd theRefMinorSuperDiag = m_KeysMatrix.matrix().diagonal(m_NumOfKeys-3*m_NumOfModes-1);
    Eigen::ArrayXd theRefMinorSubDiag = m_KeysMatrix.matrix().diagonal(-3*m_NumOfModes-1);
	for (Eigen::ArrayXXd::Index i = 0; i < theRefMinorSuperDiag.size(); ++i)
	{
		if (i % 2 == 1) //sum over odd indices
		{
			theRelKeyDuration += theRefMajorSubDiag[i] + theRefMinorSuperDiag[i];
		}
	}
	for (Eigen::ArrayXXd::Index i = 0; i < theRefMinorSubDiag.size(); ++i)
	{
		if (i % 2 == 0) //sum over even indices
		{
			theRelKeyDuration += theRefMajorSuperDiag[i] + theRefMinorSubDiag[i];
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


const double KeyEvaluationStats::getChromaticKeys() const
{
    return m_KeysMatrix.matrix().diagonal(5*m_NumOfModes).sum() +
        m_KeysMatrix.matrix().diagonal(-5*m_NumOfModes).sum() +
        m_KeysMatrix.matrix().diagonal(m_NumOfKeys-5*m_NumOfModes).sum() +
        m_KeysMatrix.matrix().diagonal(-m_NumOfKeys+5*m_NumOfModes).sum();
}

const Eigen::ArrayXXd::Index KeyEvaluationStats::getNumOfUniquesInRef() const
{
	return (m_KeysMatrix > 0.).rowwise().any().count();
}

const Eigen::ArrayXXd::Index KeyEvaluationStats::getNumOfUniquesInTest() const
{
	return (m_KeysMatrix > 0.).colwise().any().count();
}

const Eigen::ArrayXXd KeyEvaluationStats::getCorrectKeysPerMode() const
{
    Eigen::ArrayXXd outCorrectKeysPerMode = Eigen::ArrayXXd::Zero(m_NumOfModes, 2);
    for (Eigen::ArrayXXd::Index iMode = 0; iMode < m_NumOfModes; ++iMode)
    {
        for (Eigen::ArrayXXd::Index iChroma = 0; iChroma < m_NumOfChromas; ++iChroma)
        {
            // Correct keys per mode
            outCorrectKeysPerMode(iMode,0) += m_ConfusionMatrix(iChroma*m_NumOfModes+iMode, iChroma*m_NumOfModes+iMode);
            // Total duration per mode
            outCorrectKeysPerMode(iMode,1) += m_ConfusionMatrix.row(iChroma*m_NumOfModes+iMode).sum();
        }
    }
    return outCorrectKeysPerMode;
}
