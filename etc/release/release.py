#!/usr/bin/python3
import argparse,os,os.path,sys,stat,subprocess,pipes,time,shutil,multiprocessing,datetime,time

##########################################################################
##########################################################################

BUILD_FOLDER="build"
FOLDER_PREFIX="_Rel."
INTERMEDIATE_FOLDER="_Rel"

##########################################################################
##########################################################################

class Platform:
    def __init__(self,_name):
        self._name=_name

    @property
    def name(self): return self._name

# map sys.platform name to platform info
PLATFORMS={
    "win32":Platform("win32"),
    "darwin":Platform("osx"),
    "linux":Platform("linux")
}

##########################################################################
##########################################################################

g_verbose=False

def v(str):
    global g_verbose
    
    if g_verbose:
        sys.stdout.write(str)
        sys.stdout.flush()

##########################################################################
##########################################################################

class ChangeDirectory:
    def __init__(self,path):
        self._oldcwd=os.getcwd()
        self._newcwd=path

    def __enter__(self): os.chdir(self._newcwd)

    def __exit__(self,*args): os.chdir(self._oldcwd)

##########################################################################
##########################################################################

def fatal(str):
    sys.stderr.write("FATAL: %s"%str)
    if str[-1]!='\n': sys.stderr.write("\n")

    if os.getenv("EMACS") is not None: raise RuntimeError
    else: sys.exit(1)

##########################################################################
##########################################################################

def run(argv,ignore_errors=False):
    print(80*"-")
    print(" ".join([pipes.quote(x) for x in argv]))
    print(80*"-")

    ret=subprocess.call(argv)

    if not ignore_errors:
        if ret!=0: fatal("process failed: %s"%argv)

def capture(argv):
    v("Run: %s\n"%argv)
    process=subprocess.Popen(args=argv,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    output=process.communicate()
    if process.returncode!=0: fatal("process failed")
    return output[0].decode('utf8').splitlines()

def bool_str(x): return "YES" if x else "NO"

def makedirs(x):
    if not os.path.isdir(x): os.makedirs(x)

def rm(x):
    if os.path.isfile(x): os.unlink(x)

def set_file_timestamps(options,fname):
    if options.timestamp is None: return

    if os.path.islink(fname):
        # There's a link to /Applications in the dmg, and this is a
        # lame way of avoiding touching it.
        return

    t=time.mktime(options.timestamp.timetuple())

    try:
        os.utime(fname,(t,t))
    except:
        print("WARNING: failed to set timestamps for: %s"%fname,file=sys.stderr)
        pass

def set_tree_timestamps(options,root):
    for dirpath,dirnames,filenames in os.walk(root):
        for f in dirnames+filenames:
            set_file_timestamps(options,os.path.join(dirpath,f))
        
##########################################################################
##########################################################################
    
def create_intermediate_folder():
    ifolder=os.path.abspath(os.path.join(BUILD_FOLDER,
                                         INTERMEDIATE_FOLDER,
                                         PLATFORMS[sys.platform].name))

    if os.path.isdir(ifolder): shutil.rmtree(ifolder)

    makedirs(ifolder)

    return ifolder

def create_README(options,folder,rev_hash):
    with open(os.path.join(folder,"README.txt"),"wt") as f:
        f.write("b2 - a BBC Micro emulator - %s\n\n"%options.release_name)
        f.write("For licence information, please consult LICENCE.txt.\n\n")
        f.write("Documentation can be found here: https://github.com/tom-seddon/b2/blob/%s/README.md\n\n"%rev_hash)

##########################################################################
##########################################################################

def get_win32_build_folder(options):
    return '%s/%s%s'%(BUILD_FOLDER,
                      FOLDER_PREFIX,
                      options.toolchain)

def build_win32_config(timings,options,config,colour):
    folder=get_win32_build_folder(options)
    
    start_time=time.process_time()
    run(["cmd","/c",
         "color","%x"%colour],ignore_errors=True)
    
    if not options.skip_compile:
        run(["cmd","/c",
             r"etc\release\build_windows.bat",
             "/maxcpucount",
             "/property:MultiProcessorCompilation=true", # http://stackoverflow.com/a/17719445/1618406
             "/property:Configuration=%s"%config,
             "/verbosity:minimal",
             os.path.join(folder,"b2.sln")])

    if not options.skip_ctest:
        with ChangeDirectory(folder):
            run(["ctest",
                 "-j",os.getenv("NUMBER_OF_PROCESSORS"),
                 "-C",config])
            
    run(["cmd","/c",
         "color"],ignore_errors=True)

    timings[config]=time.process_time()-start_time

def build_win32(options,ifolder,rev_hash):
    timings={}

    # path that the ZIP contents will be assembled into.
    zip_folder=os.path.join(ifolder,"b2")
    makedirs(zip_folder)

    if not options.skip_debug:
        build_win32_config(timings,options,"RelWithDebInfo",0xf0)
        
    build_win32_config(timings,options,"Final",0xf4)

    print(timings)

    build64=get_win32_build_folder(options)

    shutil.copyfile(os.path.join(build64,
                                 "src/b2/Final/b2.exe"),
                    os.path.join(zip_folder,
                                 "b2.exe"))

    if not options.skip_debug:
        shutil.copyfile(os.path.join(build64,
                                     "src/b2/RelWithDebInfo/b2.exe"),
                        os.path.join(zip_folder,
                                     "b2_Debug.exe"))

    # Copy the assets from any output folder... they're all the same.
    shutil.copytree(os.path.join(build64,"src/b2/Final/assets"),
                    os.path.join(zip_folder,"assets"))
    
    shutil.copyfile("./etc/release/LICENCE.txt",
                    os.path.join(zip_folder,"LICENCE.txt"))

    shutil.copyfile(os.path.join(build64,"src/b2/RelWithDebInfo/WinPixEventRuntime.dll"),
                    os.path.join(zip_folder,"WinPixEventRuntime.dll"))

    create_README(options,zip_folder,rev_hash)

    zip_fname="b2-windows-%s.zip"%options.release_name
    zip_fname=os.path.join(ifolder,zip_fname)

    set_tree_timestamps(options,ifolder)

    # The ZipFile module is a bit annoying to use.
    with ChangeDirectory(ifolder): run(["7z.exe","a",zip_fname,"b2"])

    set_file_timestamps(options,zip_fname)
    
##########################################################################
##########################################################################

def get_darwin_build_path(config_name,path=None):
    result="%s/%s%s.%s"%(BUILD_FOLDER,
                         FOLDER_PREFIX,
                         config_name,
                         PLATFORMS[sys.platform].name)

    if path is not None: result=os.path.join(result,path)
    
    return result

def build_darwin_config(options,config):
    path=get_darwin_build_path(config)
    with ChangeDirectory(path):
        if not options.skip_compile:
            run(["ninja"])

        if not options.skip_dylib_bundler:
            run(['./submodules/macdylibbundler/dylibbundler',
                 '--create-dir',
                 '--bundle-deps',
                 '--fix-file','./src/b2/b2.app/Contents/MacOS/b2',
                 '--dest-dir','./src/b2/b2.app/Contents/libs/'])

        if not options.skip_ctest:
            run(["ctest",
                 "-j",str(multiprocessing.cpu_count())])

def copy_darwin_app(config,mount,app_name):
    dest=os.path.join(mount,app_name)
    run(["ditto",get_darwin_build_path(config,"src/b2/b2.app"),dest])

def build_darwin(options,ifolder,rev_hash):
    if not options.skip_debug: build_darwin_config(options,"r")
    build_darwin_config(options,"f")

    stem="b2-osx-"+options.release_name
    
    temp_dmg=os.path.join(ifolder,stem+"_temp.dmg")
    final_dmg=os.path.join(ifolder,stem+".dmg")
    mount=os.path.join(ifolder,stem+".dmg")

    # Copy template DMG to temp DMG.
    shutil.copyfile("./etc/release/template.dmg",temp_dmg)

    # Resize temp DMG.
    run(['hdiutil','resize','-size','500m',temp_dmg])

    # Mount temp DMG.
    run(["hdiutil","attach",temp_dmg,"-mountpoint",mount])

    try:
        # Copy text files to the DMG.
        shutil.copyfile("./etc/release/LICENCE.txt",
                        os.path.join(mount,"LICENCE.txt"))
        create_README(options,mount,rev_hash)

        # Copy app folders to the DMG.
        copy_darwin_app("f",mount,"b2.app")

        if not options.skip_debug:
            copy_darwin_app("r",mount,"b2 Debug.app")

        set_tree_timestamps(options,mount)
        
        # Give the DMG a better volume name.
        run(["diskutil","rename",mount,stem])
    finally:
        # Unmount the DMG
        run(["hdiutil","detach",mount])

    # Convert temp DMG into final DMG.
    run(["hdiutil","convert",temp_dmg,"-format","UDBZ","-o",final_dmg])
    set_file_timestamps(options,final_dmg)

    # Delete temp DMG.
    rm(temp_dmg)

##########################################################################
##########################################################################

def get_linux_build_path(config):
    return '%s/%s%s.%s'%(BUILD_FOLDER,
                         FOLDER_PREFIX,
                         config,
                         PLATFORMS[sys.platform].name)

def build_linux_config(options,config):
    with ChangeDirectory(get_linux_build_path(config)):
        if not options.skip_compile: run(['ninja'])

        if not options.skip_ctest:
            run(['ctest','-j',str(multiprocessing.cpu_count())])

def build_linux(options,ifolder,rev_hash):
    if not options.skip_debug: build_linux_config(options,'r')
    build_linux_config(options,'f')
    
##########################################################################
##########################################################################
        
def main(options):
    global g_verbose
    g_verbose=options.verbose

    rev_hash=capture(["git","rev-parse","HEAD"])[0]

    # TODO: this is a bit ugly! But the options.toolchain dependency
    # makes it a bit fiddly to have it data-driven.
    if PLATFORMS[sys.platform].name=="win32":
        init_target='init_%s'%options.toolchain
        build_fun=build_win32
    elif PLATFORMS[sys.platform].name=="osx":
        init_target='init'
        build_fun=build_darwin
    elif PLATFORMS[sys.platform].name=="linux":
        init_target='init'
        build_fun=build_linux

    if not options.skip_cmake:
        # the -j is worth doing. the init steps for ninja builds can
        # run each configuration's copy of cmake in parallel, for a
        # useful speedup.
        #
        # the output is a bit difficult to read, but by the time this
        # script is run there ought not to be any need to debug
        # things, hopefully.
        run([options.make,
             "-j%d"%options.make_jobs,
             init_target,
             "FOLDER_PREFIX=%s"%FOLDER_PREFIX,
             "RELEASE_MODE=1",
             "RELEASE_NAME=%s"%options.release_name])

    ifolder=create_intermediate_folder()

    build_fun(options,ifolder,rev_hash)

##########################################################################
##########################################################################

def timestamp(x): return datetime.datetime.strptime(x,"%Y%m%d-%H%M%S")
        
if __name__=="__main__":
    parser=argparse.ArgumentParser()

    if sys.platform not in PLATFORMS: fatal("unsupported platform: %s"%sys.platform)
    if sys.platform=="win32": default_make="bin/snmake.exe"
    else: default_make="make"

    parser.add_argument("-v","--verbose",action="store_true",help="be more verbose")
    parser.add_argument("--skip-cmake",action="store_true",help="skip the cmake step")
    parser.add_argument("--skip-compile",action="store_true",help="skip the compile step")
    parser.add_argument("--skip-ctest",action="store_true",help="skip the ctest step")
    parser.add_argument("--skip-debug",action="store_true",help="skip any debug build that might be built")
    parser.add_argument("--skip-dylib-bundler",action="store_true",help="skip dylibbundler when building for macOS")
    parser.add_argument("--make",metavar="FILE",dest="make",default=default_make,help="use %(metavar)s as GNU make. Default: ``%(default)s''")
    parser.add_argument('--make-jobs',metavar='N',default=multiprocessing.cpu_count(),type=int,help='run %(metavar)s GNU make jobs at once. Default: %(default)d')
    parser.add_argument("--timestamp",metavar="TIMESTAMP",dest="timestamp",default=None,type=timestamp,help="set files' atime/mtime to %(metavar)s - format must be YYYYMMDD-HHMMSS")
    parser.add_argument("release_name",metavar="NAME",help="name for release. Embedded into executable, and used to generate output file name")
    
    if sys.platform=='win32':
        vsver=os.getenv('VisualStudioVersion')
        if vsver is not None: vsver=int(float(vsver))

        if vsver==14: toolchain='vs2015'
        elif vsver==15: toolchain='vs2017'
        elif vsver==16: toolchain='vs2019'
        else: toolchain=None

        if toolchain is None: help_suffix=''
        else: help_suffix="Default: ``%s''"%toolchain
        
        parser.add_argument('-t','--toolchain',
                            metavar='TOOLCHAIN',
                            default=toolchain,
                            required=toolchain is None,
                            help='toolchain to use. One of: vs2015, vs2017, vs2019. '+help_suffix)
    else:
        parser.add_argument('-t','--toolchain',
                            metavar='TOOLCHAIN',
                            help='placeholder, ignored')
    
    main(parser.parse_args(sys.argv[1:]))
