#ifndef NoteEvaluationStats_h
#define NoteEvaluationStats_h

//============================================================================
/**
	Class representing .

	@author		Johan Pauwels
	@date		20101206
*/
//============================================================================
#include <Eigen/Core>

class NoteEvaluationStats
{
public:

	/** Default constructor. */
	/**	
		@param	inConfusionMatrix	Confusion matrix with annotated values in the columns 
									and predicted values in the rows */
	NoteEvaluationStats(const Eigen::ArrayXXd& inConfusionMatrix);

	/** Destructor. */
	virtual ~NoteEvaluationStats();

	/**	Gets the duration in seconds of the correctly detected notes.
		@return	 */
	const double getCorrectNotes() const;
	const double getCorrectNoNotes() const;
	const double getNoteDeletions() const;
	const double getNoteInsertions() const;

	const double getOctaveErrors() const;
	const double getFifthErrors() const;
	const double getChromaticUpErrors() const;
	const double getChromaticDownErrors() const;

protected:


private:

	Eigen::ArrayXXd m_ConfusionMatrix;
	Eigen::Index m_NumOfNotes;
	Eigen::Block<Eigen::ArrayXXd> m_NotesMatrix;

};

#endif	// #ifndef NoteEvaluationStats_h
