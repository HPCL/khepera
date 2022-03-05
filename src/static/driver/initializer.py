import os 
from typing import Tuple, List, Dict, Union  
from passRunner import PassRunner, create_tool_dirs 
import multiprocessing 
from multiprocessing import Pool 




def handleInit(initL : Union[List[str], str], args : Dict[str, str]) -> Tuple[str]: 
    if isinstance(initL, list):
        if len(initL) == 0: # normal init case 
            return create_tool_dirs() 
        if len(initL) == 1: # func_only init case 
            if initL[0] == args['diff_funcs_only']:
                return create_tool_dirs(func_only=True) 

    if isinstance(initL, str): 
        return create_tool_dirs() 


def handleInitWithPasses(initL : Union[List[str], str], args : Dict[str, str]): 
    runner = None 
    count  = multiprocessing.cpu_count() 
    pool   = Pool(processes=count)
    handleInit(initL, args)

    ast_passes = [] 

    if isinstance(initL, list):
        if len(initL) == 0: 
            runner = PassRunner(initL, cg_pass=True)

        if len(initL) == 1: 
            if initL[0] == args['diff_funcs_only']: 
                runner = PassRunner(initL, func_pass=True, cg_pass=False)

    if isinstance(initL, str):  
        ast_passes_str = args['ast_pass']
        ast_passes     = ast_passes_str.split(',') 

        runner = PassRunner(initL, cg_pass=True, ast_passes=ast_passes)

        
    runner.run(pool, ast_passes=ast_passes) 
    runner.post_process_pass() 

    
    
