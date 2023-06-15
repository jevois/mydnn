// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JeVois Smart Embedded Machine Vision Toolkit - Copyright (C) 2016 by Laurent Itti, the University of Southern
// California (USC), and iLab at USC. See http://iLab.usc.edu and http://jevois.org for information about this project.
//
// This file is part of the JeVois Smart Embedded Machine Vision Toolkit.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
// Foundation, version 2.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.  You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// Contact information: Laurent Itti - 3641 Watt Way, HNB-07A - Los Angeles, CA 90089-2520 - USA.
// Tel: +1 213 740 3527 - itti@pollux.usc.edu - http://iLab.usc.edu - http://jevois.org
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file */

#include <jevois/Core/Module.H>
#include <jevois/Debug/Timer.H>
#include <jevois/Image/RawImageOps.H>
#include <jevois/DNN/Pipeline.H>

// icon from opencv

//! Detect and recognize multiple objects in scenes using OpenCV, NPU, TPU, or VPU Deep Neural Nets
/*! This module is a copy of the DNN module in the main JeVois software distribution.

    It is here for you to modify so that you can then cross-compile it and install it with your modifications on your
    JeVois camera. See README for compilation instructions.

    @author Laurent Itti

    @displayname MyDNN
    @videomapping JVUI 0 0 30.0 CropScale=RGB24@1024x576:YUYV 1920 1080 30.0 JeVois MyDNN
    @email itti\@usc.edu
    @address University of Southern California, HNB-07A, 3641 Watt Way, Los Angeles, CA 90089-2520, USA
    @copyright Copyright (C) 2023 by Laurent Itti, iLab and the University of Southern California
    @mainurl http://jevois.org
    @supporturl http://jevois.org/doc
    @otherurl http://iLab.usc.edu
    @license GPL v3
    @distribution Unrestricted
    @restrictions None
    \ingroup modules */
class MyDNN : public jevois::StdModule
{
  public: 
    // ####################################################################################################
    //! Constructor
    // ####################################################################################################
    MyDNN(std::string const & instance) : jevois::StdModule(instance)
    {
      itsPipeline = addSubComponent<jevois::dnn::Pipeline>("pipeline");
    }

    // ####################################################################################################
    //! Virtual destructor for safe inheritance
    // ####################################################################################################
    virtual ~MyDNN()
    { }

    // ####################################################################################################
    //! Processing function implementation
    // ####################################################################################################
    void doprocess(jevois::InputFrame const & inframe, jevois::RawImage * outimg,
                   jevois::OptGUIhelper * helper, bool idle)
    {
      // If we have a second (scaled) image, assume this is the one we want to process:
      jevois::RawImage const inimg = inframe.getp();

      // Ok, process it:
      itsPipeline->process(inimg, this, outimg, helper, idle);
    }
    
    // ####################################################################################################
    //! Processing function, no video output
    // ####################################################################################################
    virtual void process(jevois::InputFrame && inframe) override
    {
      doprocess(inframe, nullptr, nullptr, false);
    }
    
    // ####################################################################################################
    //! Processing function with video output to USB on JeVois-A33
    // ####################################################################################################
    virtual void process(jevois::InputFrame && inframe, jevois::OutputFrame && outframe) override
    {
      // Get the input frame:
      jevois::RawImage const & inimg = inframe.get();
      unsigned int const w = inimg.width, h = inimg.height;

      // Get the output image:
      jevois::RawImage outimg = outframe.get();

      // Input and output sizes and format must match:
      outimg.require("output", w, h, inimg.fmt);

      // Copy input to output:
      jevois::rawimage::paste(inimg, outimg, 0, 0);

      // Process and draw any results (e.g., detected boxes) into outimg:
      doprocess(inframe, &outimg, nullptr, false);

      // Send the output image with our processing results to the host over USB:
      outframe.send();
    }

#ifdef JEVOIS_PRO
    // ####################################################################################################
    //! Processing function with zero-copy and GUI on JeVois-Pro
    // ####################################################################################################
    virtual void process(jevois::InputFrame && inframe, jevois::GUIhelper & helper) override
    {
      // Compute overall frame rate, CPU usage, etc:
      static jevois::Timer timer("main", 20, LOG_DEBUG);
      std::string const & fpscpu = timer.stop();
      timer.start();
      
      // Start the display frame: winw, winh will be set by startFrame() to the display size, e.g., 1920x1080
      unsigned short winw, winh;
      bool idle = helper.startFrame(winw, winh);

      // Display the camera input frame: if all zeros, x, y, w, h will be set by drawInputFrame() so as to show the
      // video frame as large as possible and centered within the display (of size winw,winh)
      int x = 0, y = 0; unsigned short w = 0, h = 0;
      helper.drawInputFrame("c", inframe, x, y, w, h, true);

      // Process and draw any results (e.g., detected boxes) as OpenGL overlay:
      doprocess(inframe, nullptr, &helper, idle);

      // You can add ImGUI elements as you wish, here are some examples (they will disappear after the mouse and
      // keyboard have been idle for a while):
      if (idle == false)
      {
        // To draw things on top of input video but behind ImGui windows, use ImGui global background draw list.
        // Here we draw a red square that follows the mouse cursor and is behind other drawings or windows:
        auto dlb = ImGui::GetBackgroundDrawList(); // or use GetForegroundDrawList to draw in front of ImGui
        ImVec2 const p = ImGui::GetMousePos();
        dlb->AddRect(ImVec2(p.x-30, p.y-30), ImVec2(p.x+30, p.y+30), ImColor(255, 0, 0, 255) /* red */);
      
        // Just draw a simple ImGui window that shows fps if we are not idle:
        ImGuiIO & io = ImGui::GetIO();
        if (ImGui::Begin("JeVois-Pro MyDNN Module"))
        {
          ImGui::Text("Framerate: %3.2f fps", io.Framerate);
          static bool coolmode = true; // static variable has persistent value, initialized only once at program start
          ImGui::Checkbox("Cool mode (for demo only, does nothing)", &coolmode);
          ImGui::Text("Cool mode is %s", coolmode ? "on" : "off");
          ImGui::Separator();
          ImGui::Text("Click [Open ImGui Demo] under the");
          ImGui::Text("Tweaks tab to see many more widgets!");
        }
        ImGui::End();

        // To draw things on top of input video and on top of ImGui windows, use ImGui global foregound draw list:
        // Here we draw a green circle that follows the mouse cursor and is in front of other drawings or windows:
        auto dlf = ImGui::GetForegroundDrawList();
        dlf->AddCircle(ImVec2(p.x, p.y), 20, ImColor(0, 255, 0, 128) /* semi transparent green */);
      }
      
      // Show overall frame rate, CPU, camera resolution, and display resolution, at bottom of screen:
      helper.iinfo(inframe, fpscpu, winw, winh);
      
      // Render the image and GUI:
      helper.endFrame();
    }
#endif
    
    // ####################################################################################################
  protected:
    std::shared_ptr<jevois::dnn::Pipeline> itsPipeline;
};

// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(MyDNN);
