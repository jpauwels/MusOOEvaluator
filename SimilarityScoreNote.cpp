//============================================================================
/**
	Implementation file for SimilarityScoreNote.h

	@author		Johan Pauwels
	@date		20101206
*/
//============================================================================

// Includes
#include <stdexcept>
#include "SimilarityScoreNote.h"

using std::vector;
using std::string;

using namespace MusOO;

SimilarityScoreNote::SimilarityScoreNote(const std::string& inScoreSelect)
{
	if (inScoreSelect.compare("Piano") == 0)
	{
		this->m_NumOfRefLabels = 89;
		this->m_NumOfTestLabels = 89;
		m_LowestNote = 21;
	}
	else
	{
		throw std::invalid_argument("Unknown score selector: " + inScoreSelect);
	}
	this->m_Labels.resize(this->m_NumOfRefLabels);
	for (size_t i = 0; i+1 < this->m_NumOfRefLabels; ++i)
	{
		m_Labels[i] = NoteMidi(m_LowestNote.number()+i);
	}
	this->m_Labels.back() = Note::silence();
}

SimilarityScoreNote::~SimilarityScoreNote()
{
	// Nothing to do...
}

const double SimilarityScoreNote::score(const Note& inRef, const Note& inTest)
{
	this->m_RefIndex = NoteMidi(inRef).number() - m_LowestNote.number();
	this->m_TestIndex = NoteMidi(inTest).number() - m_LowestNote.number();

	if (this->m_RefIndex == this->m_TestIndex)
	{
		return 1.;
	}
	else
	{
		return 0.;
	}
}
