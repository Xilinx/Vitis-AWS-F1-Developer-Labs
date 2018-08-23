# SDAccel AWS F1 Developer Labs


Welcome to the SDAccel AWS F1 Developer Labs. By going through these labs you will gain hands-on experience with AWS F1 and learn how to develop accelerated applications using the Xilinx SDAccel development environment.

### Prerequisites

In order to run the SDAccel AWS F1 Developer Labs, you will need the following:

<details>
<summary><strong>An AWS account</strong> <i>(expand for details)</i></summary><p>

If you do not already have an Amazon Web Services (AWS) account, create one here: [https://aws.amazon.com/](https://aws.amazon.com)
<p></details><br>
<details>
<summary><strong>Access to AWS F1 instances</strong> <i>(expand for details)</i></summary><p>

AWS users need to request access to use F1 instances. Here are the steps to do so:

* Open the Service Limit Increase form: [http://aws.amazon.com/contact-us/ec2-request](http://aws.amazon.com/contact-us/ec2-request)
* Make sure your account name is correct
* Submit a 'Service Limit Increase' for 'EC2 Instances'
* Select the region where you want to access F1 instances: US East (N.Virginia), US West (Oregon) or EU (Ireland)
* Select 'f1.2xlarge' as the primary instance type
* Set the 'New limit value' to 1 or more
* Fill the rest of the form as appropriate and click 'Submit'

Requests are typically processed by AWS in 24 to 48 hours.
<p></details><br>

### Usage Instructions

These steps explain how start an EC2 F1 instance starting from the FPGA Developer AMI and setting it up to connect via a remote desktop client. 

#### Launch an EC2 instance 
1. Navigate to the AWS EC2 dashboard: [https://console.aws.amazon.com/ec2](https://console.aws.amazon.com/ec2)
1. In the top right corner, select a region with F1 instances: US East (N.Virginia), US West (Oregon) or EU (Ireland) 
1. Click **Launch Instance**

#### Choose the FPGA Developer AMI 1.3.4
1. Click **AMIs** (on the left pane, under "IMAGES")
1. Set the pulldown of the search text field to "Public images"
1. Enter "FPGA" in the text field and press "enter"
1. Select the **FPGA Developer AMI - 1.3.4...** AMI name
1. Click **Launch**

#### Choose an F1 instance
1. In the **All Instance Type** menu select "FPGA Instances"
1. Select **f1.2xlarge**

#### Enable RDP traffic to enable a remote desktop client to access the F1 instance
1. At the top of the console, select **Configure Security Groups** 
1. Click **Add Rule**
1. Select **RDP** from the pulldown menu
1. Select **My IP** from the Source pulldown

#### Review and launch
1. Click **Review and Launch**
1. Click **Launch**
1. Select an existing key pair or create a new key pair
1. Monitor the launch, click on the "View Instances" button
1. Click the instance name to display it in the instance view
1. Copy or write down the **IPv4 Public IP** address of the instance.

When the status of the newly launched instance switches to green (Running), you are ready to connect to it.
  * Allow about 10 seconds for the instance to get in the 'running' state. 
  * If needed, click the **Refresh** icon (![Refresh](./images/setup/refresh2.png?raw=true)) in the top-right corner of the EC2 Console to update the instance status information.

#### Connect to your instance via a ssh to comple the setup
1. Click **Review and Launch**
1. Click **Launch**
1. Select an existing key pair or create a new key pair
1. Monitor the launch, click on the "View Instances" button
1. Click the instance name to display it in the instance view
1. Copy or write down the **IPv4 Public IP** address of the instance.
1. Using that IP address, connect to your instance using SSH
1. Once connected to the shell, make sure the AMI version is 1.3.4
1. On that screen copy and paste to execute /home/centos/src/scripts/setup_gui.sh

#### Connect to your instance using a remote desktop client

The instance you just started is preconfigured with remote desktop protocol (RDP) services.

1. From your local machine, start a remote desktop protocol client
    - On Windows: press the Windows key and type "remote desktop".
      - You should see the "Remote Desktop Connection" in the list of programs.
      - Alternatively you can also simply type `mstsc.exe` in the Windows run prompt.
    - On Linux: RDP clients such a Remmina or Vinagre are suitable.
    - On macOS: use the Microsoft Remote Desktop v8.0.43 (that version offers color depth settings) from the Mac App Store.
1. **IMPORTANT**: Set your RDP client to use **24-bit for color depth**
    - On Windows: In the bottom-left corner of connection prompt, click Options, then select the Display tab and set Colors to True Colors (24 bit)
1. In the RDP client, enter the **IPv4 Public IP** of your instance.
1. Click **Connect**. This should bring up a message about connection certificates. 
1. Click **Yes** to dismiss the message. The Remote Desktop Connection window opens with a login prompt.
1. Login with the following credentials:
    - User: **centos**
    - Password: **sdaccel_labs**   
1. Click **Ok**.

You should now be connected to the remote F1 instance running Centos 7 and the SDAccel AWS F1 Developer Labs AMI.

#### Open the lab instructions on the remote F1 instance

1. In the remote instance, open **Firefox** from the **Applications** menu (located in top left corner of the desktop)
    - As it opens, the browser automatically loads this README file.
1. Continue following the lab instructions from within the remote instance and the web browser.
    - We suggest you perform all your copy-paste from the instructions to the shell within the RDP session to avoid issues.

You instance is now configured to run the SDAccel Developer labs on AWS F1 and you are ready to start the first module.

---------------------------------------

<p align="center"><b>
Start the first module: <a href="./modules/module_01/README.md">Introduction to the SDAccel Flow</a>
</b></p>
