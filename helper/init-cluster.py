#!/usr/bin/python

from __future__ import print_function
from string import Template
import uuid
import sys,os,time
import base64
import requests
import getopt

common_temp = '''
ip = "${EXTERNAL_IP}";
manager_addr = "${MGR_IP}:${MGR_REST_PORT}";
log = {
    path = "/opt/hvs/var/log/hvs.log";
    level = ${LOG_LEVEL};
};
'''

ioproxy_temp = '''
ioproxy = {
    uuid = "${IOP_UUID}";
    cid = "${CID}";
    scher = 6;
    data_path = "/opt/hvs/var/data/";
    data_port = ${IOP_UDP_PORT};
    data_buffer = 1024000;
    data_conn = 50;
    rpc_port = ${IOP_TCP_PORT};
    rpc_workers = 6;
    rpc_timeout = 3000;
    rpc_retry = 1;
};
'''

mgr_temp = '''
manager = {
    //use linux command uuid to generate your own uuid
    uid = "${MGR_UID}";
    cid = "${CID}";
    data_path = "/opt/hvs/var/data/";
    port = ${MGR_REST_PORT};
    thread_num = 6;
    couchbase_addr = "${COUCHBASE_IP}:${COUCHBASE_PORT}";
    couchbase_user = "${COUCHBASE_USER}";
    couchbase_passwd = "${COUCHBASE_PASS}";
    bucket = "${COUCHBASE_BUCKET}";
};
'''

client_temp = '''
client = {
    mountpoint = "${MNT_POINT}";
    listen_port = 6666;
    foreground = true;
    debug = false;
    multithread = true;
    multi_channel = 1;
    fuse_workers = 10;
    auto_unmount = true;
    use_udt = false;
    data_port_begin = 9096;
    data_port_end = 9150;
    data_buffer = 102400;
    max_queue = 12800000;
    onlink = 64;
    async = true;
    readahead = 20;
    seq_threshold = 10;
    stat_cache_size = 1000;
};
'''

config_ops = {
    'EXTERNAL_IP': '127.0.0.1',
    'CID': '1',
    'MGR_IP': "127.0.0.1",
    'MGR_REST_PORT': '9090',
    'LOG_LEVEL': '10',
    'IOP_UUID': 1,
    'IOP_TCP_PORT': '9092',
    'IOP_UDP_PORT': '9095',
    'MGR_UID': '1',
    'COUCHBASE_IP': '127.0.0.1',
    'COUCHBASE_PORT':'8091',
    'COUCHBASE_USER': 'dev',
    'COUCHBASE_PASS': 'buaaica',
    'COUCHBASE_BUCKET': 'test',
    'MNT_POINT': '/mnt/hvs',
    'CENTER_NAME': 'Beijing',
}

mgr_yn = False
iop_yn = False
cli_yn = False
couchbase_op_setted = False

def query_yn(query):
    while True:
        q_yn = raw_input(query + " [Yy/Nn]: ")
        if (q_yn == 'Y' or q_yn == 'y'):
            return True
        elif (q_yn == 'N' or q_yn == 'n'):
            return False
        else:
            print("please input [Yy/Nn]!")
            continue

def query_str(query, default = ''):
    q_str = raw_input(query + " (default: '%s') [String/ENTER]: "%default)
    if (q_str == ''):
        print('Use default value "%s"'%default)
        q_str = default
    return q_str

def initial_mgr():
    pass

def couchbse_options():
    global couchbase_op_setted
    if not couchbase_op_setted:
        config_ops['COUCHBASE_IP'] = query_str("Input Couchbase Address", config_ops['COUCHBASE_IP'])
        config_ops['COUCHBASE_PORT'] = query_str("Input Couchbase Listening Port", config_ops['COUCHBASE_PORT'])
        config_ops['COUCHBASE_USER'] = query_str("Input Couchbase Username", config_ops['COUCHBASE_USER'])
        config_ops['COUCHBASE_PASS'] = query_str("Input Couchbase Password", config_ops['COUCHBASE_PASS'])
        config_ops['COUCHBASE_BUCKET'] = query_str("Input Couchbase Bucket", config_ops['COUCHBASE_BUCKET'])
    couchbase_op_setted = True

def interact_options():
    global mgr_yn, iop_yn, cli_yn
    print("******Start GVDS Config Phase...*******")
    mgr_yn = query_yn("Start manager daemon?")
    iop_yn = query_yn("Start io_proxy daemon?")
    cli_yn = query_yn("Mount fuse client?")
    if mgr_yn or iop_yn or cli_yn:
        config_ops['MGR_IP'] = query_str("Input Manager IP Address", config_ops['MGR_IP'])
        config_ops['MGR_REST_PORT'] = query_str("Input Manager HTTP Port", config_ops['MGR_REST_PORT'])
    if mgr_yn or iop_yn :
        config_ops['EXTERNAL_IP'] = query_str("Input External IP Address:", config_ops['EXTERNAL_IP'])
        config_ops['CID'] = query_str("Input Center ID:", config_ops['CID'])
    if mgr_yn:
        config_ops['MGR_UID'] = query_str("Input Manager UID", config_ops['CID'])
        couchbse_options()
    if iop_yn:
        config_ops['IOP_UUID'] = query_str("Input IOProxy UUID", uuid.uuid1())
        config_ops['IOP_TCP_PORT'] = query_str("Input IOProxy TCP Port", config_ops['IOP_TCP_PORT'])
        config_ops['IOP_UDP_PORT'] = query_str("Input IOProxy UDP Port", config_ops['IOP_UDP_PORT'])
    if cli_yn:
        config_ops['MNT_POINT'] = query_str("Input Client Mount Point", config_ops['MNT_POINT'])

    config_content = ""
    if mgr_yn or iop_yn or cli_yn:
        config_content += Template(common_temp).substitute(config_ops)
    if mgr_yn:
        mgr_config = Template(mgr_temp).substitute(config_ops)
        config_content += mgr_config
    if iop_yn:
        iop_config = Template(ioproxy_temp).substitute(config_ops)
        config_content += iop_config
    if cli_yn:
        client_config = Template(client_temp).substitute(config_ops)
        config_content += client_config
    print("======START Generated config file content START======")
    print(config_content)
    print("======END Generated config file content END======")
    config_path = query_str("Input Config File Generate Path", "/opt/hvs/hvs.conf")
    with open(config_path,'w') as f:
        f.write(config_content)
    print("Success in generating config file at %s"%config_path)
    print("******End GVDS Config Phase.******")

def setup_couchbase():
    print("******Start Couchbase Setup Phase...*******")
    couchbse_options()
    authorization = base64.b64encode(config_ops['COUCHBASE_USER'] + ":" + config_ops['COUCHBASE_PASS'])
    headers = {
        'Authorization': 'Basic %s'%authorization,
    }
    couchbase_addr = "http://%s:%s"%(config_ops['COUCHBASE_IP'], config_ops['COUCHBASE_PORT'])
    if query_yn("Should rebuild couchbase cluster at %s?"%config_ops['COUCHBASE_IP']):
        if query_yn("Should Removing Couchbase old config data and restart service?"):
            os.remove("/opt/couchbase/var/lib/couchbase/config/config.dat")
            os.system("systemctl restart couchbase-server.service")
            while True:
                print("Waiting couchbase-server's service start")
                time.sleep(3)
                try:
                    res = requests.get('%s'%(couchbase_addr), headers=headers)
                except Exception,e:
                    res = None
                if (not res or res.status_code!= 200) and query_yn("Cannot connect to couchbase. Still Wait?"):
                    continue
                elif res and res.status_code == 200:
                    break
                else:
                    exit()
        data = {
            'path':'/opt/couchbase/var/lib/couchbase/data',
            'index_path': '/opt/couchbase/var/lib/couchbase/data',
            'cbas_path': '/opt/couchbase/var/lib/couchbase/data',
        }
        res = requests.post('%s/nodes/self/controller/settings'%couchbase_addr, data=data , headers=headers)
        print(res.status_code, res.content)
        ext_ip = query_str("Config couchbase's external ip", config_ops['COUCHBASE_IP'])
        data = {'hostname':ext_ip}
        res = requests.post('%s/node/controller/rename'%couchbase_addr, data=data , headers=headers)
        print(res.status_code, res.content)
        data = {'services':'kv,n1ql,index,fts'}
        res = requests.post('%s/node/controller/setupServices'%couchbase_addr, data=data , headers=headers)
        print(res.status_code, res.content)
        data = {'password': config_ops['COUCHBASE_PASS'], 'username': config_ops['COUCHBASE_USER'], 'port': 'SAME'}
        res = requests.post('%s/settings/web'%couchbase_addr, data=data , headers=headers)
        print(res.status_code, res.content)
    if query_yn("Should rebuild couchbase bucket %s?"%config_ops['COUCHBASE_BUCKET']):
        bk_mem = query_str("Input Bucket memory quoto(MB).", "2048")
        data = {
            'flushEnabled':'1', 'threadsNumber':"3", 'replicaIndex':'0', 'replicaNumber':'0', 'evictionPolicy': 'valueOnly',
            'ramQuotaMB':bk_mem , 'bucketType':'couchbase', 'name': config_ops['COUCHBASE_BUCKET']
            }
        res = requests.post('%s/pools/default/buckets'%couchbase_addr, data=data , headers=headers)
        print("Sleep 3 seconds waiting for buckect create completing")
        time.sleep(3)
        data = {
            'indexerThreads':'0', 'logLevel':'error', 'maxRollbackPoints':'5', 'memorySnapshotInterval': '200', 'stableSnapshotInterval': '5000',
            'storageMode': 'forestdb'
        }
        res = requests.post('%s/settings/indexes'%couchbase_addr, data=data , headers=headers)
        print("Sleep 3 seconds waiting for buckect index completing")
        time.sleep(3)
        data = {
            'statement': 'CREATE PRIMARY INDEX `%s-index` on `%s` USING GSI;'%(config_ops['COUCHBASE_BUCKET'],config_ops['COUCHBASE_BUCKET'])
        }
        res = requests.post('http://%s:8093/query/service'%config_ops['COUCHBASE_IP'], data=data , headers=headers)
        if res.status_code == 200:
            print("Build bucket and n1ql index success!")
        else:
            print("failed to build bucket")
    print("******End Couchbase Setup Phase.******")
    # init cluster


def check_couchbase():
    couchbse_options()
    authorization = base64.b64encode(config_ops['COUCHBASE_USER'] + ":" + config_ops['COUCHBASE_PASS'])
    headers = {
        'Authorization': 'Basic %s'%authorization,
    }
    couchbase_addr = "http://%s:%s"%(config_ops['COUCHBASE_IP'], config_ops['COUCHBASE_PORT'])
    try:
        res = requests.get('%s/pools/default/buckets/%s'%(couchbase_addr, config_ops['COUCHBASE_BUCKET']), headers=headers)
        if res.status_code == 401:
            print("ERROR: Wrong username or password to access couchbase: %s:%s"%(config_ops['COUCHBASE_USER'],config_ops['COUCHBASE_PASS']))
            return False
        elif res.status_code == 404:
            print("ERROR: couchbase bucket %s not exists"%(config_ops['COUCHBASE_BUCKET']))
            return False
        return True
    except Exception,e :
        print("ERROR: cannot connect to couchbase %s: %s"%(config_ops['COUCHBASE_IP'], e), file=sys.stderr)
        return False

def generate_user(cn_name):
    if query_yn("generating 100 local system users, continue?"):
        for i in range(1, 101):
            os.system("useradd -r test%s"%i)
    print("Generating account document of %s in couchbase"%cn_name)
    authorization = base64.b64encode(config_ops['COUCHBASE_USER'] + ":" + config_ops['COUCHBASE_PASS'])
    headers = {
        'Authorization': 'Basic %s'%authorization,
    }
    couchbase_addr = "http://%s:%s"%(config_ops['COUCHBASE_IP'], config_ops['COUCHBASE_PORT'])
    try:
        res = requests.get('%s/pools/default/buckets/%s'%(couchbase_addr, config_ops['COUCHBASE_BUCKET']), headers=headers)
    except:
        pass

def main():
    if len(sys.argv) < 2 or sys.argv[1] == "-h" or sys.argv[1] == "--help" or \
        sys.argv[1] not in ('config','setup_db','start','init','all'):
        print("Usage: init-cluster.py [config|setup_db|start|init|all]")
        exit()
    if sys.argv[1] in ('config', 'all'):
        interact_options()
    if sys.argv[1] in ('setup_db', 'all'):
        setup_couchbase()
    if sys.argv[1] in ('start', 'all'):
        print("******Start GVDS Start Daemon Phase Start*******")
        os.system("ln -sf /opt/hvs/gvds-supervisord.conf /etc/supervisor/conf.d/")
        os.system("supervisorctl reload")
        print("sleep 15s to wait supervisor totally restart")
        time.sleep(15)
        if query_yn("Should Start GVDS Manager Daemon?") and query_yn("Before start GVDS MGR daemon, "
            "you should ensure couchbase service has started and XDCR has been manually set up!! Continue?"):
            ck_cb = check_couchbase()
            os.system("supervisorctl start hvs_manager")
        if query_yn("Should Start GVDS IOProxy Daemon?"):
            os.system("supervisorctl start hvs_ioproxy")
        if query_yn("Should Mount GVDS Client Daemon?"):
            os.system("supervisorctl start hvs_client")
        print("GVDS Daemon Status: ")
        with os.popen("supervisorctl status", 'r', -1) as st:
            print(st.read())
        print("******END GVDS Start Daemon Phase END*******")
    if sys.argv[1] in ('init', 'all'):
        print("******Start GVDS Cluster Init Phase Start*******")
        config_ops['MGR_IP'] = query_str("Input Manager IP Address", config_ops['MGR_IP'])
        config_ops['MGR_REST_PORT'] = query_str("Input Manager HTTP Port", config_ops['MGR_REST_PORT'])
        config_ops['CID'] = query_str("Input Center ID:", config_ops['CID'])
        config_ops['CENTER_NAME'] = query_str("Input Center Name:", config_ops['CENTER_NAME'])
        with os.popen("/opt/hvs/bin/centermodify --centerID %s --centerIP %s --centerPort %s --centerName %s"%(
            config_ops['CID'], config_ops['MGR_IP'], config_ops['MGR_REST_PORT'], config_ops['CENTER_NAME']), 'r', -1) as st:
            print(st.read())
        generate_user(config_ops['CENTER_NAME'])
        stor_cap = query_str("Input Storage capcity(GB)", 1024)
        with os.popen("/opt/hvs/bin/rg --ci %s --cn %s --ri %s --rn local1 --tc %s --ac 0 --mgs localhost --st 1"%(
            config_ops['CID'], config_ops['CENTER_NAME'], config_ops['CID'], stor_cap), 'r', -1) as st:
            print(st.read())
        with os.popen("/opt/hvs/bin/adminsignup -u admin --pass password", 'r', -1) as st:
            print(st.read())
        with os.popen("/opt/hvs/bin/usersignup -u test --pass password", 'r', -1) as st:
            print(st.read())
        
        print("sleep 5 sec to wait normal user create...")
        time.sleep(5)
        with os.popen("/opt/hvs/bin/userlogin -u admin -p password; /opt/hvs/bin/listapply", 'r', -1) as st:
            ids = [l[4:] for l in st.read().split('\n') if l.startswith("id:")]
            for mid in ids:
                os.popen("/opt/hvs/bin/suggestion -s agree -i %s"%mid, 'r', -1).close()
        os.popen("/opt/hvs/bin/userlogin -u test -p password; /opt/hvs/bin/zregister -z zone -s space --size 100 -c %s"%
            config_ops['CENTER_NAME'], 'r', -1).close()
        print("sleep 5 sec to wait zone create...")
        time.sleep(5)
        with os.popen("/opt/hvs/bin/userlogin -u admin -p password; /opt/hvs/bin/listapply;", 'r', -1) as st:
            ids = [l[4:] for l in st.read().split('\n') if l.startswith("id:")]
            print("found ids", ids)
            for mid in ids:
                os.popen("/opt/hvs/bin/suggestion -s agree -i %s"%mid, 'r', -1).close()
        with os.popen("/opt/hvs/bin/userlogin -u test -p password;", 'r', -1) as st:
            print(st.read())
        print("INFO: Generate Admin Account admin:password")
        print("INFO: Generate Test Account test:password with new Zone")

if __name__ == '__main__':
    main()
