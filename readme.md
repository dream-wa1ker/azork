# Azork 

A GPG capable wav audio steganography tool for linux. Intended to be a project prototype for physics - Acoustics and Ultrasonics course.



## Installing the Project

1) Build the project.

```bash
make build 
# or 
make all
```

2) Run the main binary. 

This one builds the binary and then runs it. Here the build directory is not deleted and persists. 

```bash 
make run
```

3) Test the program 

This one will build the binary and then run it, then finally clean it. No build directory persists. 

```bash 
make test 
```

4) Clean the project structure 

This will remove the build directory and clean the object files. 

```bash 
make clean
```

>[!NOTE]
> I have also included a shell script called `clean.sh` for cleaning all those files from `src/` and `include/`. I have also added the `make reset` command which will reset the entire project to the baseline, same as clean.sh, but from make. 

**Arch `makepkg`**

For installing the binary system wide, if you are in an arch based repo, you need to use the `makepkg` to build and install the binary to standard paths. Run the command as given below the install it. 

```bash 
makepkg -si 
```


