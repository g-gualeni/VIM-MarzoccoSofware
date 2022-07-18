#include "Grab_ChunkImage_Universal.h"
#include <opencv2/opencv.hpp>

#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif


using namespace Pylon;

#include <pylon/BaslerUniversalInstantCamera.h>
using namespace Basler_UniversalCameraParams;

using namespace std;


class CSampleImageEventHandler : public CBaslerUniversalImageEventHandler
{
public:
    virtual void OnImageGrabbed(CBaslerUniversalInstantCamera& camera, const CBaslerUniversalGrabResultPtr& ptrGrabResult)
    {
        CIntegerParameter chunkTimestamp(ptrGrabResult->GetChunkDataNodeMap(), "ChunkTimestamp");

        if (chunkTimestamp.IsReadable())
            cout << "OnImageGrabbed: TimeStamp (Result) accessed via node map: " << chunkTimestamp.GetValue() << endl;

       
        if (ptrGrabResult->ChunkTimestamp.IsReadable())
            cout << "OnImageGrabbed: TimeStamp (Result) accessed via result member: " << ptrGrabResult->ChunkTimestamp.GetValue() << endl;
    }
};


static const uint32_t c_countOfImagesToGrab = 5;

cv::Mat Grab_ChunkImage_Universal::imageFromCamera(cv::Mat grabImage)
{
    
    int exitCode = 0;
    PylonInitialize();
    
    try
    {
        
        CBaslerUniversalInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

        
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        camera.Open();

        if (!camera.ChunkModeActive.TrySetValue(true))
        {
            throw RUNTIME_EXCEPTION( "The camera doesn't support chunk features");
        }

        camera.ChunkSelector.SetValue(ChunkSelector_Timestamp);
        camera.ChunkEnable.SetValue(true);

       if (camera.ChunkSelector.TrySetValue(ChunkSelector_Framecounter)) 
        {
            camera.ChunkEnable.SetValue(true);
        }

        camera.ChunkSelector.SetValue(ChunkSelector_PayloadCRC16);
        camera.ChunkEnable.SetValue(true);

        camera.StartGrabbing( c_countOfImagesToGrab);

        CBaslerUniversalGrabResultPtr ptrGrabResult;
        camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        cout << "GrabSucceeded: " << ptrGrabResult->GrabSucceeded() << endl;
        cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
        cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
        
        unsigned short *pImageBuffer = (unsigned short*) ptrGrabResult->GetBuffer();
        
        if (PayloadType_ChunkData != ptrGrabResult->GetPayloadType())
        {
            throw RUNTIME_EXCEPTION( "Unexpected payload type received.");
        }

        if (ptrGrabResult->HasCRC() && ptrGrabResult->CheckCRC() == false)
        {
            throw RUNTIME_EXCEPTION( "Image was damaged!");
        }

            
        if (ptrGrabResult->ChunkTimestamp.IsReadable())
        {
             cout << "TimeStamp (Result): " << ptrGrabResult->ChunkTimestamp.GetValue() << endl;
        }

        if (ptrGrabResult->ChunkFramecounter.IsReadable())
        {
             cout << "FrameCounter (Result): " << ptrGrabResult->ChunkFramecounter.GetValue() << endl;
        }
            
        cout << endl;
        
        
        cv::Mat tmpImage = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8U, pImageBuffer);
        tmpImage.copyTo(grabImage);
        
        // Disable chunk mode.
        
        camera.StopGrabbing();
        camera.ChunkModeActive.SetValue(false);
        //camera.Close();
    }
    catch (const GenericException &e)
    {
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitCode = 1;
    }
        
    PylonTerminate(); 
    return grabImage;
    
    
}
