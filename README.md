# Homemade ECG-device with Deep Learning

First of all, I collect ECG data with my homemade ECG-device which is connected to Cloud. After recording, data will be sent to Google Spread Sheet through GAS, and then I run AI built in advanced to get a diagnosis.

![overview.jpg](/img/overview.jpg)


## What I did

### 1.) Homemade ECG-device

I implemented circuit on a breadborad for 1-lead ECG and connected to M5stack.


![ecg_device.gif](/img/ecg_device.gif)

**Spesicication**
- Analog filter: Lowpass 0.3-30 Hz
- Degital filter: Notch 50/60 Hz
- Resolutin: 12bits
- Sampling rate: 500/600 Hz
- Left button: change frequency of Notch filter
- Middle button: changing saving mode on/off
- Right button: Power on/off



### 2.) AI for diagnosis

I used CNN-base model. Please check [main.ipynb](https://github.com/masa282/homemade-ecg-checker/blob/master/src/main.ipynb)

The label categories of this model

![label.png](/img/label.png)

Please check [label_description.ipynb](https://github.com/masa282/homemade-ecg-checker/blob/master/src/label_description.ipynb) 


Thank you for reading!

___
##### Reference
- [M5stack GRAY](https://docs.m5stack.com/en/core/gray)
- [Training/Testing data](https://www.kaggle.com/datasets/shayanfazeli/heartbeat)
- [ML Model](https://arxiv.org/pdf/1805.00794.pdf)
