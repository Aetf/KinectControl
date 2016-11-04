#include "stdafx.h"
#include "resource.h"
#include "KinectControlDlg.h"
#include "SkeletalViewerDlg.h"

#include <NuiApi.h>
#include <MMSystem.h>

// KinectControlDlg部分
void CKinectControlDlg::Nui_Zero()
{
	m_pThNuiProcess        = NULL;
}

HRESULT CKinectControlDlg::Nui_Init()
{
	HRESULT hr;

	m_dlgSkel.Nui_Init();

	hr = NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_SKELETON);
	if(FAILED(hr))
	{
		MessageBoxResource(IDS_ERROR_NUIINIT);
	}
	hr = NuiSkeletonTrackingEnable(m_EvNextSkeletonEvent.m_hObject, 0);
	if(FAILED(hr))
	{
		MessageBoxResource(IDS_ERROR_SKELETONTRACKING);
	}

	hr = NuiCameraElevationSetAngle( 0L );
	if(FAILED(hr))
	{
		MessageBoxResource(IDS_ERROR_SETANGLE);
	}

	m_pThNuiProcess = AfxBeginThread(AFX_THREADPROC(Nui_ProcessThread), LPVOID(this) );

	return hr;
}

void CKinectControlDlg::Nui_UnInit()
{
	if(NULL != m_EvNuiProcessStop.m_hObject)
	{
		m_EvNuiProcessStop.SetEvent();
		if(NULL != m_pThNuiProcess)
		{
			CWaitCursor waitCursor;
			::WaitForSingleObject(m_pThNuiProcess->m_hThread,INFINITE);
		}
	}

	NuiShutdown();
}

DWORD WINAPI CKinectControlDlg::Nui_ProcessThread(LPVOID lpParam)
{
	CKinectControlDlg* pthis = (CKinectControlDlg*)lpParam;
	return pthis->Nui_ProcessThreadPriv();
}

DWORD WINAPI CKinectControlDlg::Nui_ProcessThreadPriv()
{
	const int numEvents = 3;

	HANDLE hEvents[numEvents] = {m_EvNuiProcessStop.m_hObject, m_EvNextSkeletonEvent.m_hObject, m_EvNextDepthFrameEvent.m_hObject};
	int nEventIdx;

	bool continueProcessing = true;
	while(continueProcessing)
	{
		nEventIdx = ::WaitForMultipleObjects(numEvents,hEvents, FALSE, 100);
		 // Timed out, continue
        if ( nEventIdx == WAIT_TIMEOUT )
        {
            continue;
        }

        // stop event was signalled 
        if ( WAIT_OBJECT_0 == nEventIdx )
        {
            continueProcessing = false;
            break;
        }

		// Wait for each object individually with a 0 timeout to make sure to
        // process all signalled objects if multiple objects were signalled
        // this loop iteration

        // In situations where perfect correspondance between color/depth/skeleton
        // is essential, a priority queue should be used to service the item
        // which has been updated the longest ago

		if ( WAIT_OBJECT_0 == WaitForSingleObject( m_EvNextDepthFrameEvent.m_hObject, 0 ) )
        {
			++m_dlgSkel.m_FramesTotal;
        }

		if (  WAIT_OBJECT_0 == WaitForSingleObject( m_EvNextSkeletonEvent.m_hObject, 0 ) )
        {
            Nui_GotSkeletonAlert( );
        }

		// SetFPS
		m_dlgSkel.SetFPS();

		// Blank the skeleton panel if we haven't found a skeleton recently
		if(m_bShowSkeleton)
		{
			CWnd *pWnd = m_dlgSkel.GetDlgItem(IDC_SKELETALVIEW);
			m_dlgSkel.Nui_BlankSkeletonScreen(pWnd, true);
		}
	}
	return 0;
}

void CKinectControlDlg::Nui_GotSkeletonAlert()
{
	NUI_SKELETON_FRAME SkeletonFrame = {0};

	bool bFoundSkeleton = false;
	if(SUCCEEDED(NuiSkeletonGetNextFrame(0, &SkeletonFrame)))
	{
		for(int i = 0; i < NUI_SKELETON_COUNT; i++)
		{
			if(SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED) 
			{
				bFoundSkeleton = true;
			}
		}
	}

	if(!bFoundSkeleton)
		return;

	// Found a skeleton, re-start the skeletal timer
	m_dlgSkel.Nui_RestartTimer();

	// draw skeleton
	if(m_bShowSkeleton)
		m_dlgSkel.Nui_Draw(&SkeletonFrame);

	// Control Input
	if(m_bMeasureBody)
		MyMersureBody(&SkeletonFrame);
	if(m_bControl)
		MyInputControl(&SkeletonFrame);

	UpdateTrackedSkeletons(SkeletonFrame);
}

// 选择追踪哪个骨骼并且追踪它
void CKinectControlDlg::UpdateTrackedSkeletons( const NUI_SKELETON_FRAME & skel )
{
    DWORD nearestIDs[2] = { 0, 0 };
    float nearestDistance = 1000.0f;

    // Purge old sticky skeleton IDs, if the user has left the frame, etc
    bool stickyIDFound = false;
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
        NUI_SKELETON_TRACKING_STATE trackingState = skel.SkeletonData[i].eTrackingState;

        if ( trackingState == NUI_SKELETON_TRACKED || trackingState == NUI_SKELETON_POSITION_ONLY )
        {
            if ( skel.SkeletonData[i].dwTrackingID == m_StickySkeletonId )
            {
                stickyIDFound = true;
            }
        }
    }

    if ( !stickyIDFound )
    {
        m_StickySkeletonId = 0;
    }

    // Calculate nearest and sticky skeletons
    for ( int i = 0 ; i < NUI_SKELETON_COUNT; i++ )
    {
		const NUI_SKELETON_DATA & skeleton = skel.SkeletonData[i];

		if ( skeleton.eTrackingState != NUI_SKELETON_NOT_TRACKED )
        {
            // Save SkeletonIds for sticky mode if there's none already saved
            if ( 0 == m_StickySkeletonId )
            {
                m_StickySkeletonId = skel.SkeletonData[i].dwTrackingID;
            }

            // calculate the skeleton's position on the screen

			if ( skeleton.Position.z < nearestDistance )
            {
				nearestDistance = skeleton.Position.z;
                nearestIDs[0] = skeleton.dwTrackingID;
            }
        }
    }

	HRESULT hr;
#ifdef _DEBUG
	static int count = 0;
#endif
    if ( SV_TRACKED_SKELETONS_NEAREST == m_TrackingMode )
    {
        hr = NuiSkeletonSetTrackedSkeletons(nearestIDs);
    }
    if ( SV_TRACKED_SKELETONS_STICKY == m_TrackingMode )
    {
        DWORD stickyIDs[2] = { m_StickySkeletonId, 0 };

        hr = NuiSkeletonSetTrackedSkeletons(stickyIDs);
    }
#ifdef _DEBUG
	if(FAILED(hr))
		TRACE("\nSet Tracked Skeletons Failed %d\n",count++);
#endif
}

/// <summary>
/// Sets or clears the specified skeleton tracking flag
/// </summary>
/// <param name="flag">flag to set or clear</param>
/// <param name="earse">true to clear, false to set</param>
void CKinectControlDlg::UpdateSkeletonTrackingFlag(DWORD flag, bool earse)
{
	DWORD newFlags = m_SkeletonTrackingFlags;
	if(earse)
		newFlags &= ~flag;
	else
		newFlags |= flag;

	m_SkeletonTrackingFlags = newFlags;
	NuiSkeletonTrackingEnable( m_EvNextSkeletonEvent.m_hObject, m_SkeletonTrackingFlags);
}

/// <summary>
/// Invoked when the user changes the selection of tracked skeletons
/// </summary>
/// <param name="mode">skelton tracking mode to switch to</param>
void CKinectControlDlg::UpdateTrackedSkeletonSelection(int mode)
{
	m_TrackingMode = mode;
	UpdateSkeletonTrackingFlag(NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS,
		(mode == SV_TRACKED_SKELETONS_DEFAULT));
}

// SkeletalViewerDlg部分

static const COLORREF g_JointColorTable[NUI_SKELETON_POSITION_COUNT] = 
{
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_HIP_CENTER
    RGB(169, 176, 155), // NUI_SKELETON_POSITION_SPINE
    RGB(168, 230, 29), // NUI_SKELETON_POSITION_SHOULDER_CENTER
    RGB(200, 0,   0), // NUI_SKELETON_POSITION_HEAD
    RGB(79,  84,  33), // NUI_SKELETON_POSITION_SHOULDER_LEFT
    RGB(84,  33,  42), // NUI_SKELETON_POSITION_ELBOW_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_WRIST_LEFT
    RGB(215,  86, 0), // NUI_SKELETON_POSITION_HAND_LEFT
    RGB(33,  79,  84), // NUI_SKELETON_POSITION_SHOULDER_RIGHT
    RGB(33,  33,  84), // NUI_SKELETON_POSITION_ELBOW_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_WRIST_RIGHT
    RGB(37,   69, 243), // NUI_SKELETON_POSITION_HAND_RIGHT
    RGB(77,  109, 243), // NUI_SKELETON_POSITION_HIP_LEFT
    RGB(69,  33,  84), // NUI_SKELETON_POSITION_KNEE_LEFT
    RGB(229, 170, 122), // NUI_SKELETON_POSITION_ANKLE_LEFT
    RGB(255, 126, 0), // NUI_SKELETON_POSITION_FOOT_LEFT
    RGB(181, 165, 213), // NUI_SKELETON_POSITION_HIP_RIGHT
    RGB(71, 222,  76), // NUI_SKELETON_POSITION_KNEE_RIGHT
    RGB(245, 228, 156), // NUI_SKELETON_POSITION_ANKLE_RIGHT
    RGB(77,  109, 243) // NUI_SKELETON_POSITION_FOOT_RIGHT
};

static const COLORREF g_SkeletonColors[NUI_SKELETON_COUNT] =
{
    RGB( 255, 0, 0),
    RGB( 0, 255, 0 ),
    RGB( 64, 255, 255 ),
    RGB( 255, 255, 64 ),
    RGB( 255, 64, 255 ),
    RGB( 128, 128, 255 )
};

void CSkeletalViewerDlg::Nui_Zero()
{
	m_PensTotal			   = 6;
	ZeroMemory(m_Points,sizeof(m_Points));
	m_LastSkeletonFoundTime= 0;
	m_bScreenBlanked       = false;
}

void CSkeletalViewerDlg::Nui_Init()
{
	RECT rc;

	if(m_SkeletonBMP.GetSafeHandle() == NULL)
	{
		GetDlgItem(IDC_SKELETALVIEW)->GetWindowRect(&rc);
		CDC *pDC = GetDlgItem(IDC_SKELETALVIEW)->GetDC();
		HDC hdc = pDC->GetSafeHdc();

		int width = rc.right - rc.left;
		int height = rc.bottom - rc.top;

		m_SkeletonBMP.CreateCompatibleBitmap(pDC, width, height);
		m_SkeletonDC.CreateCompatibleDC(pDC);

		GetDlgItem(IDC_SKELETALVIEW)->ReleaseDC(pDC);
		m_SkeletonOldObj = m_SkeletonDC.SelectObject(&m_SkeletonBMP);
	}
}

void CSkeletalViewerDlg::Nui_UnInit()
{
	m_SkeletonDC.SelectObject(m_SkeletonOldObj);
}

void CSkeletalViewerDlg::Nui_BlankSkeletonScreen(CWnd *pWnd, bool NotUseDbBuf)
{
	if(NotUseDbBuf) // 是线程直接调用
	{
		int t = ::timeGetTime();
		if((t - m_LastSkeletonFoundTime) <= 250 || m_bScreenBlanked)
			return;
		m_bScreenBlanked = true;
	}

	CDC *pDC = NotUseDbBuf? pWnd->GetDC() : &m_SkeletonDC;
	RECT rct;
	pWnd->GetClientRect(&rct);
	pDC->PatBlt(0, 0, rct.right, rct.bottom, BLACKNESS);
	if(NotUseDbBuf)
		pWnd->ReleaseDC(pDC);
}

void CSkeletalViewerDlg::Nui_Draw(NUI_SKELETON_FRAME * SkeletonFrame)
{
	// draw each skeleton color according to the slot within they are found.
	Nui_BlankSkeletonScreen(GetDlgItem(IDC_SKELETALVIEW), false);

	for(int i = 0; i < NUI_SKELETON_COUNT; i++)
	{
		if(SkeletonFrame->SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
			SkeletonFrame->SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER] != NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			Nui_DrawSkeleton(&(SkeletonFrame->SkeletonData[i]), GetDlgItem(IDC_SKELETALVIEW), i);
		}
	}

	Nui_DoDoubleBuffer(GetDlgItem(IDC_SKELETALVIEW), &m_SkeletonDC);
}

void CSkeletalViewerDlg::Nui_DoDoubleBuffer(CWnd *pWnd, CDC * pDC)
{
	RECT rct;
	pWnd->GetClientRect(&rct);
	CDC *pdc = pWnd->GetDC();

	pdc->BitBlt(0, 0, rct.right, rct.bottom, pDC, 0, 0, SRCCOPY);
	pWnd->ReleaseDC(pdc);
}

void CSkeletalViewerDlg::Nui_DrawSkeleton(NUI_SKELETON_DATA *pSkel, CWnd *pWnd, int WhichSkeletonColor)
{
	RECT rct;
	pWnd->GetClientRect(&rct);
	int width = rct.right;
	int height = rct.bottom;
	if(m_Pen[0].GetSafeHandle() == NULL)
	{
		for(int i =0; i < m_PensTotal; i++)
		{
			m_Pen[i].CreatePen(PS_SOLID, width / 80, g_SkeletonColors[i]);
		}
	}
	USHORT depth;
	for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
	{
		NuiTransformSkeletonToDepthImage(pSkel->SkeletonPositions[i],&m_Points[i].x, &m_Points[i].y, &depth);

		m_Points[i].x = (m_Points[i].x * width) / 320;
		m_Points[i].y = (m_Points[i].y * height) / 240;
	}

	CObject* pOldObj = m_SkeletonDC.SelectObject(&m_Pen[WhichSkeletonColor % m_PensTotal]);
	
	Nui_DrawSkeletonSegment(pSkel, 4, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_HEAD);
	Nui_DrawSkeletonSegment(pSkel, 5, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT,NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);
	Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);
    Nui_DrawSkeletonSegment(pSkel,5,NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

	for(int i = 0; i < NUI_SKELETON_POSITION_COUNT; i++)
		if(pSkel->eTrackingState != NUI_SKELETON_POSITION_NOT_TRACKED)
		{
			CPen JointPen;

			JointPen.CreatePen(PS_SOLID, 9, g_JointColorTable[i]);
			m_SkeletonDC.SelectObject(&JointPen);

			m_SkeletonDC.MoveTo(m_Points[i]);
			m_SkeletonDC.LineTo(m_Points[i]);

			m_SkeletonDC.SelectObject(pOldObj);
			JointPen.DeleteObject();
		}
}

void CSkeletalViewerDlg::Nui_DrawSkeletonSegment(NUI_SKELETON_DATA *pSkel, int numJoints, ...)
{
	va_list vl;
	va_start(vl,numJoints);

	POINT segmentPositions[NUI_SKELETON_POSITION_COUNT];
	int segmentPositionsCount = 0;

	DWORD polylinePointCounts[NUI_SKELETON_POSITION_COUNT];
	int numPolylines = 0;
	int currentPointCount = 0;

	// Note the loop condition: We intentionally run one iteration beyond the
    // last element in the joint list, so we can properly end the final polyline.
	for(int iJoint = 0; iJoint <= numJoints; iJoint++)
	{
		if(iJoint < numJoints)
		{
			NUI_SKELETON_POSITION_INDEX jointIndex = va_arg(vl, NUI_SKELETON_POSITION_INDEX);

			if(pSkel->eSkeletonPositionTrackingState[jointIndex] != NUI_SKELETON_POSITION_NOT_TRACKED)
			{
				segmentPositions[segmentPositionsCount] = m_Points[jointIndex];
				segmentPositionsCount++;
				currentPointCount++;

				// Fully processed the current joint; move on to the next one
				continue;
			}
		}

		// If we fall through to here, we're either beyond the last joint, or
        // the current joint is not tracked: end the current polyline here.
		if(currentPointCount > 1)
		{
			// Current polyline already has at least two points: save the count.
			polylinePointCounts[numPolylines++] = currentPointCount;
		}
		else if(currentPointCount == 1)
		{
			// Current polyline has only one point: ignore it.
			segmentPositionsCount--;
		}
		currentPointCount = 0;
	}

	if(numPolylines > 0)
		m_SkeletonDC.PolyPolyline(segmentPositions, polylinePointCounts, numPolylines);

	va_end(vl);
}
