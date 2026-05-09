# Đưa lab lên GitHub và chạy trên Ubuntu VirtualBox

## Đẩy lên GitHub (một lần — trên máy đang có bản lab đầy đủ)

1. Trên github.com → **New repository** → để **trống** (không tick README/license), ví dụ tên `Lab07-CPUFuzzing`.

2. Trong terminal, **`cd`** vào thư mục gốc lab rồi:

```bash
git init
git add -A
git status
git commit -m "Lab07 CPU fuzzing"
git branch -M main
git remote add origin https://github.com/<USERNAME>/<REPO>.git
git push -u origin main
```

- Khi được hỏi mật khẩu trên HTTPS, dùng **Personal Access Token** (Fine-grained hoặc Classic với quyền vào repo), không phải mật khẩu GitHub web.
- Hoặc **SSH**:

```bash
git remote add origin git@github.com:<USERNAME>/<REPO>.git
```

## Ubuntu trong VirtualBox — clone và chạy Part 3

Áp dụng cho **Ubuntu 64-bit x86** (quan trọng: `Vpsp` là ELF Linux amd64).

```bash
sudo apt update
sudo apt install -y git python3 build-essential gcc-riscv64-unknown-elf binutils-riscv64-unknown-elf gdb-multiarch

cd ~
git clone https://github.com/<USERNAME>/<REPO>.git
cd Lab07-CPUFuzzing
chmod +x run.sh debug.sh sim/obj_dir/Vpsp

cd part3
make clean && make
cd ..
./run.sh part3
```

## Kiểm tra nhanh trước khi build

```bash
riscv64-unknown-elf-gcc --version
python3 --version
test -x sim/obj_dir/Vpsp && echo "Vpsp executable ok"
```

## Ubuntu 20.04 / GCC 9 (riscv64-unknown-elf-gcc 9.x)

Nếu `make` báo `unsupported ISA subset 'z'` với `-march=rv32i_zicsr`, bản repo hiện dùng `-march=rv32i` trong các `part*/Makefile` (tương thích GCC 9). **`git pull`** rồi `make clean && make` lại. Nếu chưa kéo về được, sửa tạm toàn cục: thay `rv32i_zicsr` → `rv32i` trong `CFLAGS` và `LDFLAGS1` của từng `part*/Makefile`.

## Máy ảo “để qua đêm vẫn không ra dòng mới”

- **Ubuntu tự sleep / suspend** làm **dừng hẳn** `Vpsp`. Vào **Settings → Power** (Gnome): đặt **Suspend** = **Never**, **Screen blank** tùy ý. Hoặc terminal:
  ```bash
  sudo systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target
  ```
- **VirtualBox**: đừng bấm **Pause** máy ảo; host ngủ đôi khi cũng pause VM — chỉnh nguồn/chính sách nguồn host nếu cần.
- Nếu **đã tắt ngủ** mà vài phút vẫn chỉ thấy `running first probe*`, simulator vẫn có thể **rất chậm**; xem **CPU của process `Vpsp`** trong VM (`top`) có ~100% 1 core không — nếu có thì vẫn đang chạy.

## Debug: trace trap (xem có vào được exception handler không)

Trong `part3`:

```bash
make clean
make EXTRA_CFLAGS=-DPART3_TRAP_TRACE
cd ..
./run.sh part3
```

Sẽ in tối đa 32 dòng `[trap] #N mcause=0x...`. **Không có dòng `[trap]`** sau thời gian chờ dài → **chưa từng vào trap** (kẹt trước lần exception đầu hoặc VM không chạy). **Có `[trap]`** → handler chạy, chỉ cần kiên nhẫn thêm cho `trial done`.
