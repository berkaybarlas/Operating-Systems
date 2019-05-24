# Project 3 

### We implemented all parts of the project.

A file in the file system is a link to an inode.
A hard link creates another file with a link to the same underlying inode.

When you delete a file it removes one link to the underlying inode. The inode is only deleted when all links to the inode have been deleted.

The inode is a data structure in a Unix-style file system which describes a filesystem object such as a file or a directory.

unlink() deletes a name from the file system. If that name was the last link to a file and no processes have the file open the file is deleted and the space it was using is made available for reuse.
If the name was the last link to a file but any processes still have the file open the file will remain in existence until the last file descriptor referring to it is closed.

If the name referred to a symbolic link the link is removed.

If the name referred to a socket, fifo or device the name for it is removed but processes which have the object open may continue to use it.

remove() deletes a name from the file system. It calls unlink(2) for files, and rmdir(2) for directories.
If the removed name was the last link to a file and no processes have the file open, the file is deleted and the space it was using is made available for reuse.

If the name was the last link to a file, but any processes still have the file open, the file will remain in existence until the last file descriptor referring to it is closed.

If the name referred to a symbolic link, the link is removed.

If the name referred to a socket, FIFO, or device, the name is removed, but processes which have the object open may continue to use it.

a) text1.txt and text2.txt hard link
What are the inode values of file1.txt and file2.txt? Are they the same or different? 
* Inode values of file1.txt and file2.txt are 15217390 and both them has same value. 
Do the two files have the same—or different— contents?
* The contents of file1.txt and file2.txt are same.

Next, edit file2.txt and change its contents. After you have done so, examine the contents of file1.txt . Are the contents of file1.txt and file2.txt the same or different?
* After the contents of file2.txt changed, the contents of file1.txt changed too. Their contents are still same.

rm file1.txt

Does file2.txt still exist as well?
* After removing file1.txt, file2.txt is still exists. 

b) text3.txt and text4.txt soft link
Are the inodes the same, or is each unique? 
* Inode values of text3.txt and text4.txt are not same. text3.txt has inode value of 15217391. text4.txt has inode value of 15217646.
Next, edit the contents of file4.txt. Have the contents of file3.txt been altered as well? 
* After the contents of file4.txt changed, the contents of file3.txt changed too. Their contents are still same.

Last, delete file3.txt. After you have done so, explain what happens when you attempt to edit file4.txt.
* After the deletion of file3.txt, the contents of file4.txt is cleared. 
* After editing file4.txt, a file named file3.txt is created with the same content of file3.txt

### The project can be test with following command  
``` 
make run 
```

