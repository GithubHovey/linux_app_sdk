#!/bin/bash
DEVICE="/dev/mmcblk0"
KERNEL_IMAGE="boot.img"
START_OFFSET=0xC8000
END_OFFSET=0x20C8000
BLOCK_SIZE=512  # 使用更小的块大小确保兼容性

# 计算十进制值
START_DEC=$((START_OFFSET))
END_DEC=$((END_OFFSET))
PARTITION_SIZE=$((END_DEC - START_DEC))

# 检查镜像文件
if [ ! -f "$KERNEL_IMAGE" ]; then
    echo "Error: $KERNEL_IMAGE not found!"
    exit 1
fi

# 获取文件大小（兼容BusyBox）
IMAGE_SIZE=$(wc -c < "$KERNEL_IMAGE")

# 检查大小
if [ "$IMAGE_SIZE" -gt "$PARTITION_SIZE" ]; then
    echo "Error: Image too large for partition!"
    echo "Image size: $IMAGE_SIZE bytes"
    echo "Partition size: $PARTITION_SIZE bytes"
    exit 1
fi

# 用户确认
echo "About to:"
echo "1. Erase $DEVICE from $START_OFFSET to $END_OFFSET"
echo "2. Write $KERNEL_IMAGE to $START_OFFSET"
printf "Confirm? (y/n) "
read -r answer
case "$answer" in
    [yY]) ;;
    *) echo "Aborted."; exit 0 ;;
esac

# 擦除分区
echo "Erasing partition..."
dd if=/dev/zero of="$DEVICE" bs="$BLOCK_SIZE" seek=$((START_DEC/BLOCK_SIZE)) \
   count=$((PARTITION_SIZE/BLOCK_SIZE)) conv=notrunc

# 写入镜像
echo "Writing image..."
dd if="$KERNEL_IMAGE" of="$DEVICE" bs="$BLOCK_SIZE" seek=$((START_DEC/BLOCK_SIZE)) \
   conv=notrunc


echo "Done."
