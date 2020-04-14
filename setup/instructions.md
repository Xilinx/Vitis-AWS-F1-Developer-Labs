# Setup Instructions


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

Requests are typically processed in 24 to 48 hours.
<p></details><br>

### Launching, Configuring and Connecting to an F1 Instance

The following steps explain how to launch an EC2 F1 instance starting from the FPGA Developer AMI and setting it up to connect via a remote desktop client. 

#### Launch an EC2 F1 instance with RDP connection enabled 
1. Navigate to the [AWS Marketplace](https://aws.amazon.com/marketplace/pp/B06VVYBLZZ?qid=1585105385966&sr=0-1&ref_=srh_res_product_title)
1. Click on **Continue to Subscribe**
1. Accept the EULA and click **Continue to Configuration**
1. Select version v1.8.x and a region with F1 instances: US East (N.Virginia), US West (Oregon) or EU (Ireland)
1. Click on **Continue to Launch**
1. Select **Launch through EC2** in the **Choose Action** drop-down and click **Launch**
1. Select **f1.2xlarge** Instance type
1. At the top of the console, click on **4. Add Storage** 
1. Set size of the Root Device to `100GiB`
1. At the top of the console, click on **6. Configure Security Groups** 
1. Click **Add Rule** ( Note : Add a new rule dont modify existing rule )
    1. Select **Custom TCP Rule** from the **Type** pull-down menu
    1. Type `8443` in the **Port Range** field
    1. Select **Anywhere** from the **Source** pull-down
1. Click **Review and Launch**. This brings up the review page.
1. Click **Launch** to launch your instance.
1. Select a valid key pair and **check** the acknowledge box at the bottom of the dialog
1. Select **Launch Instances**. This brings up the launch status page
1. When ready, select **View Instances** at the bottom of the page

#### Connect to your EC2 F1 instance
1. When the status of the newly launched instance switches to green (Running), you are ready to connect to it.
    - Allow about 10 seconds for the instance to get in the 'running' state. 
    - If needed, click the **Refresh** icon (![Refresh](../images/setup/refresh2.png?raw=true)) in the top-right corner of the EC2 Console to update the instance status information.
1. In the AWS EC2 dashboard, select your instance to display its information in the bottom pane
1. Copy or write down the **IPv4 Public IP** address of the instance.
1. Using that IP address, connect to your instance using SSH (Linux) or PuTTY (Windows)
    ```sh
    ssh -i <AWS key pairs.pem> centos@<IPv4 Public IP of EC2 instance> 22 
    ```
    ```sh
    putty -i <AWS key pairs.ppk> -ssh centos@<IPv4 Public IP of EC2 instance> 22 
    ```
1. An ASCII art message welcomes you to your instance

##### Connect to your EC2 F1 from Windows Using PuTTy
If you are working from windows and getting error in the while doing ssh from a terminal using .pem file. Follow the steps in last section untilyou get **IPv4 Public IP** address of the instance. After this please follow the instruction provided on the following page to convert .pem file to .ppk and connect using PuTTY
https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/putty.h

#### Installing a GUI Desktop

The FPGA Developer AMI doesn't include a GUI Desktop, but you can set it up by running the following steps:

1. [Install NICE DCV pre-requisites](https://docs.aws.amazon.com/dcv/latest/adminguide/setting-up-installing-linux-prereq.html)

   ```
   sudo yum -y install kernel-devel
   sudo yum -y groupinstall "GNOME Desktop"
   sudo yum -y install glx-utils
   ```

1. [Install NICE DCV Server](https://docs.aws.amazon.com/dcv/latest/adminguide/setting-up-installing-linux-server.html)

   ```
   sudo rpm --import https://s3-eu-west-1.amazonaws.com/nice-dcv-publish/NICE-GPG-KEY
   wget https://d1uj6qtbmh3dt5.cloudfront.net/2019.0/Servers/nice-dcv-2019.0-7318-el7.tgz
   tar xvf nice-dcv-2019.0-7318-el7.tgz
   cd nice-dcv-2019.0-7318-el7
   sudo yum -y install nice-dcv-server-2019.0.7318-1.el7.x86_64.rpm
   sudo yum -y install nice-xdcv-2019.0.224-1.el7.x86_64.rpm

   sudo systemctl enable dcvserver
   sudo systemctl start dcvserver
   ```

1. Setup Password

   ```
   sudo passwd centos
   ```

1. Change firewall settings
      
   * Disable firewalld to allow all connections
   ```
   sudo systemctl stop firewalld
   sudo systemctl disable firewalld
   ```

1. Create a virtual session to connect to    
   
   **NOTE: You will have to create a new session if you restart your instance.** 

   ```
   dcv create-session --type virtual --user centos centos
   ```

1. Connect to the DCV Remote Desktop session

    1. **Using a web browser**
    
       * Make sure that you are using a [supported web browser](https://docs.aws.amazon.com/dcv/latest/adminguide/what-is-dcv.html#what-is-dcv-requirements).
       
       * Use the secure URL, Public IP address, and correct port (8443) to connect. For example: `https://111.222.333.444:8443`
    
          **NOTE:** When you connect make sure you use the `https` protocol to ensure a secure connection.              

    1. **Using the NICE DCV Client**
    
       * Download and install the [DCV Client](https://download.nice-dcv.com/)
       
       * Use the Public IP address, and correct port (8443) to connect

1. Logging in should show you your new GUI Desktop

---------------------------------------

<p align="center"><b>
<a href="../README.md#module-1---introduction-to-the-sdaccel-flow">Start the SDAccel Developer Labs</a>
</b></p>
