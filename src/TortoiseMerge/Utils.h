#pragma once

/**
 * \ingroup TortoiseMerge
 *
 * Helper functions
 */
class CUtils
{
public:
	CUtils(void);
	~CUtils(void);

	/**
	 * Starts an external program to get a file with a specific revision. 
	 * \param sPath path to the file for which a specific revision is fetched
	 * \param sVersion the revision to get
	 * \param sSavePath the path to where the file version shall be saved
	 * \param hWnd the window handle of the calling app
	 * \return TRUE if successful
	 */
	static BOOL GetVersionedFile(CString sPath, CString sVersion, CString sSavePath, HWND hWnd = NULL);
};
