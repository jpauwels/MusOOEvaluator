//============================================================================
/**
	Implementation file for NoteEvaluationStats.h
	
	@author		Johan Pauwels
	@date		20101206
*/
//============================================================================

// Includes
#include "NoteEvaluationStats.h"

NoteEvaluationStats::NoteEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix)
: m_ConfusionMatrix(inConfusionMatrix), m_NumOfNotes(inConfusionMatrix.rows() - 1),
  m_NotesMatrix(m_ConfusionMatrix.topLeftCorner(m_NumOfNotes, m_NumOfNotes))
{
}

NoteEvaluationStats::~NoteEvaluationStats()
{
	// Nothing to do...
}

const double NoteEvaluationStats::getCorrectNotes() const
{
	return m_NotesMatrix.matrix().trace();
}

const double NoteEvaluationStats::getCorrectNoNotes() const
{
	return m_ConfusionMatrix(m_NumOfNotes, m_NumOfNotes);
}

const double NoteEvaluationStats::getNoteDeletions() const
{
	return m_ConfusionMatrix.rightCols<1>().head(m_NumOfNotes).sum();
}

const double NoteEvaluationStats::getNoteInsertions() const
{
	return m_ConfusionMatrix.bottomRows<1>().head(m_NumOfNotes).sum();
}

const double NoteEvaluationStats::getOctaveErrors() const
{
	double theOctaveDuration = 0.;
	for (Eigen::ArrayXXd::Index i = 12; i < m_NumOfNotes; i += 12)
	{
		theOctaveDuration += m_NotesMatrix.matrix().diagonal(i).sum() + 
			m_NotesMatrix.matrix().diagonal(-i).sum();
	}
	return theOctaveDuration;
}

const double NoteEvaluationStats::getFifthErrors() const
{
	double theFifthDuration = 0.;
	for (Eigen::ArrayXXd::Index i = 5; i < m_NumOfNotes; i += 12)
	{
		theFifthDuration += m_NotesMatrix.matrix().diagonal(i).sum() + 
			m_NotesMatrix.matrix().diagonal(-i).sum();
	}
	return theFifthDuration;
}

const double NoteEvaluationStats::getChromaticUpErrors() const
{
	return m_NotesMatrix.matrix().diagonal(-1).sum();
}

const double NoteEvaluationStats::getChromaticDownErrors() const
{
	return m_NotesMatrix.matrix().diagonal(1).sum();
}
