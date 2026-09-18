import os
import requests


GEMINI_API_KEY =""
GITHUB_TOKEN=""

#def extract_pr_details(url):

def get_pr_data(owner, repo, pr_number):

    #fetching quantiataive metrics and changed file contents from the PR
    GITHUB_TOKEN= os.getenv("")
    headers ={"Accept": "application/vnd.github.v3+json"}  

    if GITHUB_TOKEN:
        headers["Authorization"] = f"token {GITHUB_TOKEN}"
    else:
        print("Warning: No GitHub Token found. Proceeding with anonymous API limits.") 

    # Get PR meta and metrics

    pr_url = f"https://api.github.com/repos/{owner}/{repo}/pulls/{pr_number}"
    pr_res = requests.get(pr_url,headers=headers)
    pr_res.raise_for_status()
    pr_info = pr_res.json()

    # extrating informations
    metrics ={
        "additions": pr_info.get("additions", 0),
        "deletions": pr_info.get("deletions",0),
        "changed_files": pr_info.get("changed_files"),
        "total_lines": pr_info.get("additions",0) + pr_info.get("deletions",0),
        "title": pr_info.get("title",""),
        "description": pr_info.get("body","")

    }  

    #get changed files for statistical analysis

    files_url = f"https://api.github.com/repos/{owner}/{repo}/pulls/{pr_number}/files"
    files_res = requests.get(files_url,headers=headers)
    files_res.raise_for_status()

    files_data = {}
    for f in files_res.json():
        filename = f["filename"]
        if filename.endswith(('.cpp','.cc','.h','.hpp'))and f['status'] != 'removed':
            raw_url = f["raw_url"]
            raw_res = requests.get(raw_url, headers=headers)
            if raw_res.status_code == 200:
                files_data[filename] =raw_res.text

    return metrics, files_data            

    