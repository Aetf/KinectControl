#pragma once


// CSetKinectAngleDlg 对话框

class CSetKinectAngleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSetKinectAngleDlg)

public:
	CSetKinectAngleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSetKinectAngleDlg();

// 对话框数据
	enum { IDD = IDD_SET_KINECT_ANGLE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnClose();
	CSliderCtrl m_ctlSldAngle;
//	afx_msg void OnTRBNThumbPosChangingSliderAngle(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL OnInitDialog();
//	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
