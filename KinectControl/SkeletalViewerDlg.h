#pragma once
#include "resource.h"
#include <NuiApi.h>
#include <MMSystem.h>

// CSkeletalViewerDlg 对话框

class CSkeletalViewerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSkeletalViewerDlg)

public:
	CSkeletalViewerDlg(CWnd* pParent);   // 标准构造函数
	virtual ~CSkeletalViewerDlg();

// 对话框数据
	enum { IDD = IDD_SKELETALVIEWER };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()


// 对话框实现部分
public:
	void         SetWithGuide(bool withGuide);
	void         SetFPS();

private:
	bool         m_bWithGuide;
	
// Nui部分
public:
	void         Nui_Zero();
	void         Nui_Init();
	void         Nui_UnInit();
	void         Nui_BlankSkeletonScreen(CWnd *pWnd, bool getDC);
	void         Nui_RestartTimer();
	void         Nui_Draw(NUI_SKELETON_FRAME * SkeletonFrame);
	void         Nui_DoDoubleBuffer(CWnd *pWnd,CDC* pDC);
	void         Nui_DrawSkeleton(NUI_SKELETON_DATA * pSkel, CWnd *pWnd, int WhichSkeletonColor);
	void         Nui_DrawSkeletonSegment(NUI_SKELETON_DATA * pSkel, int numJoints, ... );

	int          m_FramesTotal;
private:
	CPen         m_Pen[NUI_SKELETON_COUNT];
	CDC          m_SkeletonDC;
	CBitmap      m_SkeletonBMP;
	CObject*     m_SkeletonOldObj;
	int          m_PensTotal;
	POINT        m_Points[NUI_SKELETON_POSITION_COUNT];
	int          m_LastFPStime;
	int          m_LastFramesTotal;
	int          m_LastSkeletonFoundTime;
	bool         m_bScreenBlanked;
public:
	virtual BOOL OnInitDialog();
	//CStatic m_imgGuide;
	afx_msg void OnClickedClose();
};
