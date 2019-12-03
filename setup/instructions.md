# Setup Instructions


For this workshop, an F1 instance is pre-setup for you and instance connection details should have been provided to you by the workshop support staff.

We will be using NICE DCV to start a remote desktop session to your instance.

Please feel free to reach out to any of our workshop support staff to ask questions about this setup or if you get stuck at any point!

> **NOTE:** We have seen VPN connections causing issues with connections. If you face such an issue, please re-try after turning your VPN connection off.

### Connect to your instance
To be able to connect to the instance, you have two options in order of preference:

#### Option 1. **Using the NICE DCV Client(Preferred/Stable)**

   * Download and install the [DCV Client](https://download.nice-dcv.com/) for your laptop platform.
   
   * Use the Public IP address, and port 8443 to connect.

      An example login screen (for the DCV Client you will need to connect first using the IP:Port, for example `111.222.333.444:8443`):

      ![DCV Login](../images/setup/dcv_login.png)
   * Login using the username `centos` and the password provided to you.
   * You should now be able to see your Desktop running on an F1 Instance
      ![DCV Desktop](../images/setup/dcv_desktop.png) 
                                                                                                                 
#### Option 2. **Using a web browser**

   * You can use this option if you cannot download/use the DCV Viewer.
   * Make sure that you are using a [supported web browser](https://docs.aws.amazon.com/dcv/latest/adminguide/what-is-dcv.html#what-is-dcv-requirements).
   
   * Use the secure URL, Public IP address, and port 8443 to connect. For example: `https://111.222.333.444:8443`

      > **NOTE:** When you connect make sure to use the `https` protocol.
      
   * The DCV Server uses a self signed certificate. Since your browser does not have a way of verifying the certificate, you will see warnings on your browser about an untrusted/unsecure connection.
   * This is normal and you can bypass the warnings based on your browser
   
        * Chrome
            1. Click on Advanced
            ![Chrome_Warning_1](../images/setup/chrome_dcv_cert_warning_1.png)
    
            1. Click on Proceed
            ![Chrome_Warning_2](../images/setup/chrome_dcv_cert_warning_2.png)
   
        * Firefox
            1. Click on Advanced and then click on `Accept use risk and continue`
            ![Firefox_Warning](../images/setup/firefox_dcv_cert_warning.png)

   * Use the Public IP address, and port 8443 to connect.

      An example login screen (for the DCV Client you will need to connect first using the IP:Port, for example `111.222.333.444:8443`):

      ![DCV Login](../images/setup/dcv_login.png)
   * You should now be able to see your Desktop running on an F1 Instance
        ![DCV Desktop_Browser](../images/setup/dcv_desktop_browser.png)
  

### Start your workshop

> **NOTE:** If you see a blank/blue screen, press the escape key to login again. Use the same password as you used before.
> This happens because Gnome locks the screen every 5m by default.

> To disable this from happening again, select the power settings:
>       ![DCV Desktop](../images/setup/desktop_lock_1.png)     
> Turn screen lock off:  
>       ![DCV Desktop](../images/setup/desktop_lock_2.png)   

   1. Open a terminal and follow the Workshop Module instructions.
   ![DCV Start_Terminal](../images/setup/dcv_desktop_start_terminal.png)

   1. The [SDAccel AWS F1 Labs repository](https://github.com/Xilinx/SDAccel-AWS-F1-Developer-Labs) and the [aws-fpga](htttps://github.com/aws/aws-fpga) Development Kit have already been downloaded.

   1. Check that your instance has your workshop cloned:
        ```bash
        ls -la /home/centos/src/project_data/
        ```
   1. Update the `SDAccel-AWS-F1-Developer-Labs` directory if necessary.
        ```bash
        cd /home/centos/src/project_data/SDAccel-AWS-F1-Developer-Labs && git pull -r
        ```
   1. Source the `sdaccel_setup.sh` script from the `aws-fpga` repository. This sets up environment variables as well as provides the AWS Platform file.
        ```bash
        source $AWS_FPGA_REPO_DIR/sdaccel_setup.sh
        ``` 
      > **NOTE:** You would need to source this on every new terminal you use for this workshop.

   1. Open the `F1_ReInvent_Workshop.desktop` file on the desktop and follow the workshop!
   > **NOTE** Clicking the `F1_ReInvent_Workshop.desktop` file will prompt to trust the document launcher. Click Trust and Proceed as that will open up the main README from Github.
       
   > **NOTE:** If Github is down/slow, you can use local webserver to look at the workshop material locally on the instance. Run the following command on the terminal to open the instructions on a browser: `firefox http://localhost:6419`
   1. You are all setup to start the first module!
   
---------------------------------------

<p align="center"><b>
<a href="../README.md">Start the SDAccel Workshop!</a>
</b></p>
