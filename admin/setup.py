
import os
import sys
import subprocess
import signal
import glob

admin_dir = os.path.dirname(os.path.abspath(__file__))

# Local packages to include in the Python virtualenv
packages=[('admin','acro-admin'), ('tpl','cxxtest','python'), ('packages','*','python')]
local_packages = []
for package in packages:
    tmp_ = package
    for pkg in glob.glob( os.path.join(*tmp_) ):
        local_packages.append(pkg)


def signal_handler(signum, frame):
    pid=os.getpid()
    pgid=os.getpgid(pid)
    if pgid == -1:
        sys.stderr.write("  ERROR: invalid pid %s\n" % (pid,))
        sys.exit(1)
    os.killpg(pgid,signal.SIGTERM)
    sys.exit(1)

signal.signal(signal.SIGINT,signal_handler)
if sys.platform[0:3] != "win":
    signal.signal(signal.SIGHUP, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)


def run():
    # Copy any / all setup scripts from the bootstrap directory
    rc = subprocess.call([os.path.join('.','bootstrap','bootstrap'), 'all'])
    if rc != 0:
        sys.stderr.write("ERROR bootstrapping the source tree\n")
        sys.exit(rc)

    # Set up the virtual environment (if it doesn't exist)
    trunk = (len(sys.argv) > 1 and '--trunk' in sys.argv)
    if trunk:
        sys.argv.remove('--trunk')
    rc=0
    if os.path.exists('python'):
        logfile = open('tpl/python.log', 'w+')
    else:
        logfile = open('tpl/python.log', 'w')
        cmd = [ sys.executable,
                os.path.join('bin','pyomo_install'),'--venv', 'python' ]
        if trunk:
            sys.stdout.write("Installing Python from trunk\n")
            cmd.append('--trunk')
            cmd.extend([ '--config', os.path.join(admin_dir,'vpy','dev.ini') ])
        else:
            sys.stdout.write("Installing Python from cached packages\n")
            cmd.extend(['--zip', os.path.join(admin_dir,'vpy','python.zip')])

        # Add any local packages that also need to be installed in editable mode
        cmd.extend(local_packages)

        print cmd
        sys.stdout.flush()
        rc = subprocess.call(cmd, stdout=logfile, stderr=subprocess.STDOUT)
        logfile.flush()
    if rc != 0:
        sys.stderr.write("ERROR installing Python virtual environment.  "
                         "See tpl/python.log\n")
        sys.exit(rc)


    # Set up the (autotools) build: targets include: 
    #     configure, reconfigure, build
    if len(sys.argv) > 1:
        rc = subprocess.call([
                os.path.join('.','python','bin','python'), 
                os.path.join('python','bin','driver') ] + sys.argv[1:])
    sys.exit(rc)
    
