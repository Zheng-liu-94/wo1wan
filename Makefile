# wo1wan —— 模仿 wiliwili 的 Switch 自制程序 (畅玩空间在线玩)
#
# 构建前请确保已安装 devkitPro，并且环境变量 DEVKITPRO 指向安装目录
# （默认 /opt/devkitpro，Windows 上 devkitPro 的 MSYS2 环境会自动设置）。
#
#   make            # 编译，产物在 out/wo1wan.nro
#   make DEVKITPRO=/c/devkitpro   # Windows 上手动指定路径时
#   make clean      # 清理
#
# 需要的 devkitPro 组件：devkitA64、libnx、tools
#   (dkp-pacman -S switch-dev)

DEVKITPRO ?= /opt/devkitpro

PREFIX    := $(DEVKITPRO)/devkitA64/bin/aarch64-none-elf-
CC        := $(PREFIX)gcc

LIBNX_INC := $(DEVKITPRO)/libnx/include
LIBNX_LIB := $(DEVKITPRO)/libnx/lib
SPECS     := $(DEVKITPRO)/libnx/switch.specs
ELF2NRO   := $(DEVKITPRO)/tools/bin/elf2nro
NACPTOOL  := $(DEVKITPRO)/tools/bin/nacptool

TARGET    := wo1wan
BUILD     := build
OUT       := out

APP_TITLE   := wo1wan
APP_AUTHOR  := wiliwili-fork
APP_VERSION := 1.0.0
ICON        := icon.jpg

ARCH   := -march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE
CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) -I$(LIBNX_INC)
LDFLAGS := -specs=$(SPECS) -g $(ARCH) -Wl,-Map,$(BUILD)/$(TARGET).map
LIBS   := -L$(LIBNX_LIB) -lnx

OBJS := $(BUILD)/main.o

.PHONY: all clean

all: $(OUT)/$(TARGET).nro

$(BUILD):
	@mkdir -p $(BUILD)

$(OUT):
	@mkdir -p $(OUT)

$(BUILD)/main.o: source/main.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(TARGET).nacp:
	$(NACPTOOL) --create --language=en \
		--title="$(APP_TITLE)" --author="$(APP_AUTHOR)" \
		--version="$(APP_VERSION)" $@

$(OUT)/$(TARGET).nro: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).nacp $(ICON) | $(OUT)
	$(ELF2NRO) $(BUILD)/$(TARGET).elf $@ --icon=$(ICON) --nacp=$(BUILD)/$(TARGET).nacp

clean:
	@rm -rf $(BUILD) $(OUT)
