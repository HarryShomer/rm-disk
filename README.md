# rm-disk

This is just a relatively simple program I wrote to delete a file's inode and overwrite it's blocks in storage with random sequences of data. It also ensures the file is deleted from cache. This only works on Linux.

NOTE: This is just something I wrote for fun. DO NOT actually use this on your computer unless you are ok with possibly overwriting random sections of your disk.


## How it works

We first must determine what partition/device the file lives on. You may not know it but your computer actually has multiple partitions. This is done using the `df` command. This is the value under "FileSystem" in the output. 

```
Filesystem     1K-blocks    Used Available Use% Mounted on
/dev/sda3       18447056 4414140  13072816  26% /
```

In a Unix System everything is file. Therefore the device that the file you want to delete lives on is a file. This means we can - read the device in, find where the data for our file to delete is stored in there, write to those locations, then save our new file/device/partition. 

Knowing now how to modify the disk directly, we now need to know where to do so. How do we find out where a given file is physically located on a device? There are a few ways to do so. The one I prefer is the filefrag command. The command is helpful for telling you if a given file is fragmented (which means it's spread across multiple non-continuous locations in storage known as extents) but by doing so tells us that actual locations in memory of the file. Running `filefrag -v <file>` returns us each of those extents and the blocks they span (a block is a chunk of memory, often 4096 bytes in ext4, that an OS uses to allocate storage).

``` 
Filesystem type is: ef53
File size of lotr.txt is 108 (1 block of 4096 bytes)
ext:     logical_offset:        physical_offset: length:   expected: flags:
0:        0..       0:    1090638..   1090638:      1:             last,eof
lotr.txt: 1 extent found
```

Looking at the output of the command under physical offset, we see my file only spans one block - 1090638. So that's the only block in storage that contains the contents of this file
and what we need to overwrite. With that in mind we can now just change that location in the file (1090638 * 4096) with a random sequence of 4096 bytes. 

It's now tempting to think that we just need to delete the inode and we are done. But that's not true. The reason why is because the contents of the file is still located in the cache. You can test this by just overwriting the file's contents but not deleting the inode. If you open the file you'll see nothing has changed. This is because it's just grabbing it from the cache and doesn't know it's been modified. So we need to delete that file from cache. This is done using the dd command. I won't go into it but here's the command.

```
dd of=<file> oflag=nocache conv=notrunc,fdatasync count=0
```

We can now delete the inode and be done.


## Requirements

- Linux Distribution (I've only tested on arch but it should be fine for most)
- C++17
- filefrag command
- sudo privilges (this will not run otherwise)
- Non-fragmented file (should only be stored on one extent)

C++17 can be had through g++. You can also install filefrag with your choice of package manager.


## Usage

Clone or download this repo to your machine. After so run `make` to create the executable. It is called `rm-disk`. It can then be run as:

```
sudo ./rm-disk <file>
```

To run this from anywhere on your machine and without the "./" just add the directory of the executable to your path.
