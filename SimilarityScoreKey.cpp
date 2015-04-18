//============================================================================
/**
	Implementation file for SimilarityScoreKey.h
	
	@author		Johan Pauwels
	@date		20100914
*/
//============================================================================

// Includes
#include <stdexcept>
	using std::runtime_error;
#include "SimilarityScoreKey.h"

using std::string;
using std::vector;

using namespace MusOO;

SimilarityScoreKey::SimilarityScoreKey(const std::string& inScoreSelect)
{
	if (!inScoreSelect.compare("Mirex"))
	{
		m_ScoreMatrix << 
			(Eigen::Array<double,1,12>() << 1., 0.5, 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.5).finished(), 
			(Eigen::Array<double,1,12>() << 0.2, 0., 0., 0.3, 0., 0., 0., 0., 0., 0., 0., 0.).finished();
	}
	else if (!inScoreSelect.compare("Binary"))
	{
		m_ScoreMatrix << 
			(Eigen::Array<double,1,12>() << 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.).finished(), 
			(Eigen::Array<double,1,12>() << 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.).finished();
	}
	else if (!inScoreSelect.compare("Lerdahl"))
	{
		m_ScoreMatrix << 
			(Eigen::Array<double,1,12>() << 0., 7., 14., 14., 16., 23., 30., 23., 16., 14., 14., 7.).finished(), 
			(Eigen::Array<double,1,12>() << 7., 14., 10., 7., 9., 16., 21., 23., 23., 21., 21., 14.).finished();
		m_ScoreMatrix /= m_ScoreMatrix.rowwise().mean().replicate(1,12);
		m_ScoreMatrix = m_ScoreMatrix.exp();
	}
	else
	{
		throw runtime_error("Unknown score selector '" + inScoreSelect + "'");
	}
	this->m_NumOfRefLabels = 25;
	this->m_NumOfTestLabels = 25;
	this->m_Labels.resize(this->m_NumOfRefLabels);
	for (size_t i = 0; i < 24; ++i)
	{
		m_Labels[i] = Key(Chroma::circleOfFifths(s_firstChroma)[i / 2],
			i%2==0?Mode::major():Mode::minorNatural());
	}
	this->m_Labels.back() = Key::silence();
}

SimilarityScoreKey::~SimilarityScoreKey()
{
	// Nothing to do...
}

const double SimilarityScoreKey::score(const Key& inRefKey, const Key& inTestKey)
{
	this->m_RefIndex = calcKeyIndex(inRefKey);
	this->m_TestIndex = calcKeyIndex(inTestKey);

	if ((this->m_RefIndex == 24 || this->m_TestIndex == 24) &&
		this->m_RefIndex != this->m_TestIndex)
	{
		return 0.;
	}
	else
	{
		//distance moving clockwise on circle of fifths from reference to test
		int theClockwiseDistance = Interval(inRefKey.tonic(), inTestKey.tonic()).circleStepsCW();
		if (inRefKey.mode().isMajor())
		{
			return m_ScoreMatrix(inTestKey.mode().isMajor()?0:1,theClockwiseDistance);
		}
		else
		{
			return m_ScoreMatrix(inTestKey.mode().isMajor()?1:0,(12-theClockwiseDistance)%12);
		}
	}
}

const size_t SimilarityScoreKey::calcKeyIndex(const Key& inKey)
{
	if (inKey == Key::silence())
	{
		return 24;
	}
	int theIndex = Interval(s_firstChroma, inKey.tonic()).circleStepsCW();
	int theMode = inKey.mode().isMajor()?0:1;
	return 2 * theIndex + theMode;
}
