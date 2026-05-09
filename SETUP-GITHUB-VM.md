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
