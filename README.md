# IoT Contiki OS Assignments (1 - 4)

Kho lưu trữ chứa mã nguồn, tệp cấu hình, firmware, kịch bản mô phỏng thực hành trên hệ điều hành Contiki OS (Tmote Sky).

---

### Hướng dẫn đồng bộ thư mục vào máy ảo Contiki

Để hệ thống build (Makefile.include) nhận đúng đường dẫn thư viện, copy các thư mục bài làm vào đường dẫn tương ứng trong thư mục 'contiki':

1. **Assignment 1:
Tạo thư mục 'my-assignments' trong thư mục:       
  '~/contiki/examples/'
Copy toàn bộ tệp trong thư mục 'assignment1/' vào thư mục:  
  '~/contiki/examples/my-assignments/'

2. **Assignment 2:
Copy toàn bộ tệp trong thư mục 'assignment2/' vào thư mục:  
  '~/contiki/examples/rime/'

3. **Assignment 3: 
Tạo thư mục 'assignment3' trong thư mục:       
  '~/contiki/examples/'
Copy toàn bộ tệp trong thư mục 'assignment3/' vào thư mục:  
  '~/contiki/examples/assignment3/'

4. **Assignment 4:
Tạo thư mục 'assignment4' trong thư mục:       
  '~/contiki/examples/'
Copy toàn bộ tệp trong thư mục 'assignment4/' vào thư mục:  
  '~/contiki/examples/assignment4/'

---

### Hướng dẫn biên dịch và mô phỏng trên Cooja

Mỗi bài tập đã được biên dịch sẵn tệp nhị phân '.sky'. Có thể nạp trực tiếp vào Cooja Simulator hoặc biên dịch lại bằng dòng lệnh Terminal:

1. **Assignment 1:
   cd ~/contiki/examples/my-assignments
   make TARGET=sky clean
   make TARGET=sky assignment1_single.sky
   make TARGET=sky assignment1_multi.sky

2. **Assignment 2:
   cd ~/contiki/examples/rime
   make TARGET=sky clean
   make TARGET=sky assignment2.sky

3. **Assignment 3:
  cd ~/contiki/examples/assignment3
  make TARGET=sky clean
  make TARGET=sky assignment3-node.sky

4. **Assignment 4:
  cd ~/contiki/examples/assignment4
  make TARGET=sky clean
  make TARGET=sky node.sky

*Khởi chạy trình mô phỏng Cooja bằng lệnh:
 cd ~/contiki/tools/cooja && ant run
