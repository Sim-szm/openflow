// Copyright (c) 2014, OpenFlow
// Author: kobemiller<kobemiller@126.com>
// Created: 2014-07-03
// Description:
// deal with tasks received from master
#include <string.h>
#include <glog/logging.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "agent_handler.h"
#include "agent_execute.h"

namespace openflow { namespace agent {
    /*
    CAgentHandler::CAgentHandler() 
    {
        execute_task_thread = boost::shared_ptr<boost::thread>
    	(new boost::thread(boost::BOOST_BIND(&CAgentHandler::execute_task, this)));
    	LOG(INFO) << "kobemiller";
    }

    CAgentHandler::~CAgentHandler() 
    {
    	execute_task_thread->join();
    }
*/
    CAgentHandler::CAgentHandler()
    {
        msg_init();
        if ( (pid = fork()) == -1 )
        {
            LOG(ERROR) << "fork error";
            exit(-1);
        }
        else if ( pid == 0 )
        {
            std::cout << "execute process" << std::endl;
            execute_task();
        }
    }
    CAgentHandler::~CAgentHandler()
    {
    }

    int32_t CAgentHandler::msg_init()
    {
        key_t MSGKEY = 2048;
        msgid = msgget(MSGKEY, IPC_CREAT|IPC_EXCL|0666);
        if ( msgid == -1 )
        {
            msgid = msgget(MSGKEY, 0);
            if (  msgid < 0 )
            {
                LOG(ERROR) << "msgget error";
                exit(-1);
            }
        }
        return 0;
    }

    bool CAgentHandler::receive_task(const openflow::task_info &task)
    {
        bool receive_rv = false;
        msg_task.msg_type = 1;
        msg_task.Task.task_id = task.task_id;
        strcpy(msg_task.Task.task_name, task.task_name.c_str());
        strcpy(msg_task.Task.cmd, task.cmd.c_str());
        if ( msgsnd(msgid, &(msg_task), sizeof(msg_task), 0) == -1 )
            LOG(INFO) << "receive failed\n", std::cout << "receive failed\n";
        else
        {
            LOG(INFO) << "receive success\n", std::cout << "receive success\n";
            receive_rv = true;
        }
                return receive_rv;
    }

    int32_t CAgentHandler::execute_task()
    {
        CTaskExecute main_execute;
        while ( 1 )
        {
        	if ( (main_execute.fork_cnt) >= (main_execute.FORK_MAX) )
        	{
        		LOG(ERROR) << "fork to the max";
        		sleep(10);
        		continue;
        	}
        	else
        	{
        		(main_execute.fork_cnt)++;
                if(msgrcv(msgid, &(msg_task), sizeof(msg_task), 0, 0) == -1 )
                {
                    LOG(ERROR) << "msgrcv error";
                    exit(-1);
                }
                 
        		int32_t re_rv = real_execute(msg_task.Task);
        		if ( re_rv != 0 )
        			LOG(ERROR) << "real_execute error";
        		else
        		 	(main_execute.fork_cnt)--;
        	}
            
        }
        return 0;
    }

    int32_t CAgentHandler::real_execute(const openflow::agent::conv_task_info &task)
    {
    	int32_t pid;
        if ( (pid = fork()) == -1 )
            LOG(ERROR) << "fork error";
        else if ( pid == 0 )
        {
        	CChild child;
        	child.handler_task(task);
            child.parent_trace_child();
            child.trace_time();
            child.get_task_status();
        }
        else if ( pid > 0 )
        {
            std::cout << "real execute" <<std::endl;
        	return 0;
        }
        return 0;
    }

} }

