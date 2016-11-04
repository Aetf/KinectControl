// SetKinectAngleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "KinectControl.h"
#include "SetKinectAngleDlg.h"
#include <NuiApi.h>
#include "afxdialogex.h"


// CSetKinectAngleDlg 对话框

IMPLEMENT_DYNAMIC(CSetKinectAngleDlg, CDialogEx)

CSetKinectAngleDlg::CSetKinectAngleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSetKinectAngleDlg::IDD, pParent)
{

}

CSetKinectAngleDlg::~CSetKinectAngleDlg()
{
}

void CSetKinectAngleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_ANGLE, m_ctlSldAngle);
}


BEGIN_MESSAGE_MAP(CSetKinectAngleDlg, CDialogEx)
//	ON_WM_CLOSE()
//	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_ANGLE, &CSetKinectAngleDlg::OnTRBNThumbPosChangingSliderAngle)
//	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CSetKinectAngleDlg 消息处理程序

BOOL CSetKinectAngleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_ctlSldAngle.SetRange(0,54);
	m_ctlSldAngle.SetPos(27);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CSetKinectAngleDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	int angle;
	angle = m_ctlSldAngle.GetPos() - 27;
	HRESULT hr;
	hr = NuiCameraElevationSetAngle(angle);
#ifdef _DEBUG
	switch(hr)
	{
	case ERROR_RETRY:
		TRACE("\nERROR_RETRY\n");
	case ERROR_TOO_MANY_CMDS:
		TRACE("\nERROR_TOO_MANY_CMDS\n");
	case E_NUI_DEVICE_NOT_READY:
		TRACE("\nE_NUI_DEVICE_NOT_READY\n");
	}
#endif

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
