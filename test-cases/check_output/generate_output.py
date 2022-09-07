# coding=utf-8

import os
import json
import sys
import time
import requests
import subprocess


'''
此脚本用于自动在 Chrome 中运行 check_output 中的 JavaScript 文件，并生成 /* OUTPUT */ 中的 console 输出
用于 RunJavaScript.outputCheck 的 Unittest 对比输出内容.
'''

HELP_MESSAGE = '''Usage: {} [-a] [filename]'''.format(os.path.basename(__file__))

TIMEOUT = 1

class GenericElement(object):
    def __init__(self, name, parent):
        self.name = name
        self.parent = parent

    def __getattr__(self, attr):
        func_name = '{}.{}'.format(self.name, attr)

        def generic_function(**args):
            self.parent.pop_messages()
            self.parent.message_counter += 1
            # message_id = int('{}{}'.format(id(self), self.parent.message_counter))
            message_id = self.parent.message_counter
            call_obj = {'id': message_id, 'method': func_name, 'params': args}
            self.parent.ws.send(json.dumps(call_obj))
            result, _ = self.parent.wait_result(message_id)
            return result
        return generic_function


class ChromeInterface(object):
    message_counter = 0

    def __init__(self, host='localhost', port=9222, tab=0, timeout=TIMEOUT, auto_connect=True):
        import websocket
        # pip install websocket_client

        self.websocket = websocket

        self.host = host
        self.port = port
        self.ws = None
        self.tabs = None
        self.timeout = timeout
        if auto_connect:
            self.connect(tab=tab)


    def get_tabs(self):
        response = requests.get('http://{}:{}/json'.format(self.host, self.port))
        self.tabs = json.loads(response.text)

    def connect(self, tab=0, update_tabs=True):
        if update_tabs or self.tabs is None:
            self.get_tabs()
        wsurl = self.tabs[tab]['webSocketDebuggerUrl']
        self.close()
        self.ws = self.websocket.create_connection(wsurl)
        self.ws.settimeout(self.timeout)

    def connect_targetID(self, targetID):
        try:
            wsurl = 'ws://{}:{}/devtools/page/{}'.format(self.host, self.port, targetID)
            self.close()
            self.ws = self.websocket.create_connection(wsurl)
            self.ws.settimeout(self.timeout)
        except:
            wsurl = self.tabs[0]['webSocketDebuggerUrl']
            self.ws = self.websocket.create_connection(wsurl)
            self.ws.settimeout(self.timeout)    
        
    def close(self):
        if self.ws:
            self.ws.close()

    # Blocking
    def wait_message(self, timeout=None):
        timeout = timeout if timeout is not None else self.timeout
        self.ws.settimeout(timeout)
        try:
            message = self.ws.recv()
        except:
            return None
        finally:
            self.ws.settimeout(self.timeout)
        return json.loads(message)

    # Blocking
    def wait_event(self, event, timeout=None):
        timeout = timeout if timeout is not None else self.timeout
        start_time = time.time()
        messages = []
        matching_message = None
        while True:
            now = time.time()
            if now-start_time > timeout:
                break
            try:
                message = self.ws.recv()
                parsed_message = json.loads(message)
                messages.append(parsed_message)
                if 'method' in parsed_message and parsed_message['method'] == event:
                    matching_message = parsed_message
                    break
            except:
                break
        return (matching_message, messages)

    # Blocking
    def wait_result(self, result_id, timeout=None):
        timeout = timeout if timeout is not None else self.timeout
        start_time = time.time()
        messages = []
        matching_result = None
        while True:
            now = time.time()
            if now-start_time > timeout:
                break
            try:
                message = self.ws.recv()
                parsed_message = json.loads(message)
                messages.append(parsed_message)
                if 'result' in parsed_message and parsed_message['id'] == result_id:
                    matching_result = parsed_message
                    break
            except:
                break
        return (matching_result, messages)

    # Non Blocking
    def pop_messages(self):
        messages = []
        self.ws.settimeout(0)
        while True:
            try:
                message = self.ws.recv()
                messages.append(json.loads(message))
            except:
                break
        self.ws.settimeout(self.timeout)
        return messages

    def __getattr__(self, attr):
        genericelement = GenericElement(attr, self)
        self.__setattr__(attr, genericelement)
        return genericelement

def args_to_string(args):
    a = []
    for arg in args:
        typ = arg.get('type')
        description = arg.get('description')
        value = arg.get('value')

        if typ == 'string':
            # { "type": "string", "value": "a" }, 
            a.append(value)
        elif typ == 'boolean':
            # { "type": "boolean",  "value": true }, 
            a.append('true' if value else 'false')
        elif typ == 'number':
            # { "type": "number", "description": "1" },
            a.append(description)
        elif typ == 'symbol':
            # { "type": "symbol", "description": "Symbol(x)" }
            a.append(description)
        elif typ == 'function':
            # { "type": "function", "description": "function() { return 1}" }
            a.append(description)
        elif typ == 'undefined':
            # { "type": "undefined" }
            a.append('undefined')
        elif typ == 'object':
            subtype = arg.get('subtype')
            if subtype == 'null':
                a.append('null')
                continue
            className = arg.get('className')
            if className == 'RegExp':
                # { "description": "/a/", "className": "RegExp", "type": "object" }
                a.append(description)
            elif className == 'Array':
                # { "type": "object", "className": "Array", "preview": { "properties": [{ "type": "number", "name": "3", "value": "NaN" } ] } }
                preview = arg.get('preview')
                properties = preview.get('properties')
                a.append('[')
                a.append(', '.join([prop.get('value', '##') for prop in properties]))
                a.append(']')
            else:
                # if className == 'Object'
                # { "type": "object",  "className": "Object", "preview": {"properties": [{ "type": "number", "name": "1", "value": "NaN" }] }}
                properties = (arg.get('preview') or arg).get('properties')
                if properties is None:
                    print(arg)
                    assert(0)
                a.append('{')
                a.append(', '.join([prop.get('name') + ': ' + prop.get('value') for prop in properties]))
                a.append('}')
        else:
            print(arg)
            assert(0)

    return ' '.join(a)

class ChromeRunner(object):

    def __init__(self):
        self._process = subprocess.Popen(['/Applications/Google Chrome.app/Contents/MacOS/Google Chrome', '--remote-debugging-port=9222', '--headless']) #, '--headless'
        self._chrome = None

        for i in range(10):
            time.sleep(0.2)
            try:
                self._connect()
            except:
                pass

    def _connect(self):
        if not self._chrome:
            self._chrome = ChromeInterface()

            self._chrome.Network.enable()
            self._chrome.Page.enable()
            self._chrome.Runtime.enable()
            #chrome.Console.enable()

    def stop(self):
        self._process.terminate()

    def run_code_with_logs(self, code):
        fn = '/tmp/__test_code.html'
        with open(fn, 'wb') as fp:
            code = '<script>{}</script>'.format(code)
            fp.write(code)

        return self.open_page_with_logs('file://' + fn)

    def open_page_with_logs(self, url):
        self._connect()

        self._chrome.Page.navigate(url=url)

        logs = []
        while True:
            msg = self._chrome.wait_message(5)
            if msg is None:
                break
            method = msg.get('method')
            # print method
            if method == 'Runtime.consoleAPICalled':
                print json.dumps(msg, indent=1)
                # print '\n'.join(logs)
                args = msg['params']['args']
                line = []
                line.append(args_to_string(args))
                logs.append(' '.join(line))
            elif method == 'Page.frameStoppedLoading':
                print 'Got Page.frameStoppedLoading'
                break
        logs = '\n'.join(logs)
        # print logs
        return logs



def run_js(chrome, snipets):
    outputs = []

    for i in range(len(snipets)):
        code = snipets[i]
        print(' Run code: {} '.format(i).center(70, '='))
        print(code[:100])
        output = chrome.run_code_with_logs(code)
        print(output[:100])

        outputs.append(output)

    return outputs

def generate_output_of_file(chrome, fn):
    snipets = []

    cur_dir = os.path.abspath(os.path.dirname(__file__)) + os.path.sep
    relative_fn = fn
    if fn.startswith(cur_dir):
        relative_fn = fn[len(cur_dir):]

    print(' Generate output for file: {} '.format(relative_fn).center(80, '='))

    # 拆分为小的代码片段
    with open(fn, 'rb') as fp:
        content = fp.read()

        while content:
            left, _, right = content.partition('/* OUTPUT\n')
            if not right:
                raise Exception('NO /*OUTPUT */ string to split code: ' + content)
            left = left.strip()
            snipets.append(left)

            _, _, content = right.partition('\n*/')
            content = content.strip()

    outputs = run_js(chrome, snipets)

    print(' Save output for file: {} '.format(relative_fn).center(80, '='))

    # 生成新的文件内容
    assert(len(outputs) == len(snipets))
    a = []
    for i in range(len(snipets)):
        code = snipets[i].strip()
        if code.startswith('// Index:'):
            _, _, code = code.partition('\n')
        code = '// Index: {}\n'.format(i) + code

        a.append(code)
        a.append('/* OUTPUT')
        a.append(outputs[i])
        a.append('*/\n\n')

    for i in range(len(a)):
        s = a[i]
        if type(s) is unicode:
            a[i] = s.encode('utf-8')
    content = '\n'.join(a)
    with open(fn, 'wb') as fp:
        fp.write(content)

    print('Done saving.')

def add_all_files(files, cur_dir):
    for root, _, names in os.walk(cur_dir):
        for name in names:
            if name.endswith('.js') and not name.startswith('.'):
                fn = os.path.abspath(os.path.join(root, name))
                files.append(fn)

def main():
    cur_dir = os.path.abspath(os.path.dirname(__file__))

    if len(sys.argv) <= 1:
        print(HELP_MESSAGE)
        return

    files = []
    if sys.argv[1] == '-a' or sys.argv[1] == '--all':
        add_all_files(files, cur_dir)
    else:
        for name in sys.argv[1:]:
            files.append(os.path.abspath(name))

    chrome = ChromeRunner()

    try:
        for fn in files:
            generate_output_of_file(chrome, fn)
    finally:
        chrome.stop()

    print(' Done: processed {} files '.format(len(files)).center(80, '='))

if __name__ == '__main__':
    main()
