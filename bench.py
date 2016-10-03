import sys, os, subprocess, signal, os.path

all_programs = [
    'glib_hash_table',
    'stl_unordered_map',
    'boost_unordered_map',
    'google_sparse_hash_map',
    'google_dense_hash_map',
    'qt_qhash',
    'python_dict',
    'ruby_hash',
    'robin_hood',
    'stl_map',
    'custom',
]

programs = []

for program in all_programs:
    program_path = './build/' + program
    csv_path = program_path + '.csv'
    if not os.path.isfile(program_path):
        continue
    if os.path.isfile(csv_path):
        if os.path.getmtime(program_path) > os.path.getmtime(csv_path):
            os.remove(csv_path)
        else:
            continue
    programs.append(program)

# remove programs that have not been updated
programs = [
    p
    for p in programs
    if os.path.isfile('./build/' + p) and (not os.path.isfile('./build/' + p + '.csv') or os.path.getmtime('./build/' + p) > os.path.getmtime('./build/' + p + '.csv'))
]

minkeys  = 128
maxkeys  = 5*1000*1000
interval = 2
best_out_of = 3

# for the final run, use this:
#minkeys  =  2*1000*1000
#maxkeys  = 40*1000*1000
#interval =  2*1000*1000
#best_out_of = 3
# and use nice/ionice
# and shut down to the console
# and swapoff any swap files/partitions

if len(sys.argv) > 1:
    benchtypes = sys.argv[1:]
else:
    benchtypes = ('sequential', 'random', 'delete', 'lookup', 'sequentialstring', 'randomstring', 'deletestring', 'lookupstring')

for benchtype in benchtypes:
    nkeys = minkeys
    while nkeys <= maxkeys:
        for program in programs:
            fastest_attempt = None
            fastest_attempt_data = ''

            for attempt in range(best_out_of):
                proc = subprocess.Popen(['./build/'+program, str(nkeys), benchtype], stdout=subprocess.PIPE)

                # wait for the program to fill up memory and spit out its "ready" message
                try:
                    runtime = float(proc.stdout.readline().strip())
                except:
                    runtime = 0

                ps_proc = subprocess.Popen(['ps up %d | tail -n1' % proc.pid], shell=True, stdout=subprocess.PIPE)
                nbytes = int(ps_proc.stdout.read().split()[4]) * 1024
                ps_proc.wait()

                os.kill(proc.pid, signal.SIGKILL)
                proc.wait()

                if nbytes and runtime: # otherwise it crashed
                    line = ','.join(map(str, [benchtype, nkeys, program, nbytes, "%0.6f" % runtime]))

                    if fastest_attempt is None or runtime < fastest_attempt:
                        fastest_attempt = runtime
                        fastest_attempt_data = line

            if fastest_attempt is not None:
                print fastest_attempt_data
                with open('./build/' + program + '.csv', 'a') as f:
                    f.write(fastest_attempt_data + '\n')

        nkeys *= interval
