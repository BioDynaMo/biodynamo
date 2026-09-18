
GEMINI_API_KEY = ""  
import re
import os
from google import genai
from google.genai import types

def generate_ai_insights(metrics, files_data, complexity_warnings="None"):
    """Generates PR analysis text using Gemini 2.5 Flash."""
    file_list = "\n".join(list(files_data.keys()))
    
    
    #  THE ANALYST (Text Generation)
    # ==================----------------===============---------------===========-------------
    prompt_text01 = f"""
    You are a Staff Software Engineer analyzing a Pull Request for BioDynaMo.
    
    Quantitative Metrics:
    - Title: {metrics.get('title', 'N/A')}
    - Lines Added: {metrics.get('additions', 0)}
    - Lines Removed: {metrics.get('deletions', 0)}
    
    Files Changed:
    {file_list}
    
    High Complexity Warnings:
    {complexity_warnings}
    
    You are an expert C++ software engineer and open-source maintainer.
    Analyze the following GitHub Pull Request and write a professional technical analysis report.
    
    You must format your response exactly matching this 6-point rubric:
    PURPOSE: Purpose and Functionality
    POSITIVES: Positive Aspects.
    CONCERNS: Potential Concerns or Weaknesses and Indicators of potential architectural issues, code smells, or spaghetti code.
    CODE_QUALITY: Code Quality and Maintainability Considerations.
    
    
    """
    
    print("Generating architectural analysis text with Gemini 2.5 Flash...")
    
    #try:
        # Initialize the official Google GenAI client
        # This automatically looks for the GEMINI_API_KEY environment variable
       #client = genai.Client(api_key=GEMINI_API_KEY)
  
        
        #response = client.models.generate_content(
            #model='gemini-2.5-flash',
            #contents=prompt_text01,
            #config=types.GenerateContentConfig(
                #temperature=0.2, # Slight creativity for writing
           # ),
        #)
        #text_output01 = response.text.replace("**", "").replace("## ", "")
        
    #except Exception as e:
       # print(f" Gemini API Error: {e}")
        #return "Error", "Error", "Error", "Error", "Error", "Error"
    
    prompt_text02 = f"""
    You are a Staff Software Engineer analyzing a Pull Request for BioDynaMo.
    
    Quantitative Metrics:
    - Title: {metrics.get('title', 'N/A')}
    - Lines Added: {metrics.get('additions', 0)}
    - Lines Removed: {metrics.get('deletions', 0)}
    
    Files Changed:
    {file_list}
    
    High Complexity Warnings:
    {complexity_warnings}
    
    You are an expert C++ software engineer and open-source maintainer.
    Analyze the following GitHub Pull Request and write a professional technical analysis report.
    
    Provide your response using EXACTLY this heading:
    IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes.
    ACTION_PLAN : A prioritised list of specific recommendations for improvement.
    SUMMARY : Comprehensive explanation of the contribution overall.
    END: ending by
     
    """
    prompt_text03 = f"""

    You are a Staff Software Engineer analyzing a Pull Request for BioDynaMo.
    Analyze the following GitHub Pull Request and based on the following GitHub Pull Request analysis, create a prioritized action plan for the developer.
    
    Quantitative Metrics:
    - Title: {metrics.get('title', 'N/A')}
    - Lines Added: {metrics.get('additions', 0)}
    - Lines Removed: {metrics.get('deletions', 0)}
    
    Files Changed:
    {file_list}
    
    High Complexity Warnings:
    {complexity_warnings}

    Provide your response using EXACTLY this heading:
    SUMMARY : Comprehensive explanation of the contribution overall.
    """
    
    def response_out(ptext):
        client = genai.Client(api_key=GEMINI_API_KEY)
        response_2 = client.models.generate_content(
        model='gemini-2.5-flash',
        contents=ptext,)
        
        return response_2
    
    text_output01 = response_out(prompt_text01).text.replace("**", "").replace("## ", "")
    text_output02 =response_out(prompt_text02).text.replace("**", "").replace("## ", "")
    text_output03 =response_out(prompt_text03).text.replace("**", "").replace("## ", "")
    
    print(text_output01)
    print(text_output02)
    #print (text_output03)

    # Finally, combine them for your parser, or parse them separately!
    #text_output = text_output01 + "\n" + action_plan_text
    
    
    # PARSING ENGINE
    # ===================------------===========---------===========---------

    def extract_section(text, start_keyword, end_keywords):
        end_pattern = "|".join([f"{k}:?" for k in end_keywords]) + "|$"
        pattern = rf"{start_keyword}:?\s*(.*?)(?={end_pattern})"
        match = re.search(pattern, text, re.IGNORECASE | re.DOTALL)
        return match.group(1).strip() if match else f"Data not generated for {start_keyword}."

    try:
        purpose = extract_section(text_output01, "PURPOSE: Purpose and Functionality", ["POSITIVES: Positive Aspects", "CONCERNS: Potential Concerns or Weaknesses and Indicators of potential architectural issues, code smells, or spaghetti code", "CODE_QUALITY: Code Quality and Maintainability Considerations", "IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes"])
        positives = extract_section(text_output01, "POSITIVES: Positive Aspects", ["CONCERNS: Potential Concerns or Weaknesses and Indicators of potential architectural issues, code smells, or spaghetti code", "CODE_QUALITY: Code Quality and Maintainability Considerations", "IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes"])
        concerns = extract_section(text_output01, "CONCERNS: Potential Concerns or Weaknesses and Indicators of potential architectural issues, code smells, or spaghetti code", ["CODE_QUALITY: Code Quality and Maintainability Considerations", "IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes"])
        quality = extract_section(text_output01, "CODE_QUALITY: Code Quality and Maintainability Considerations", ["IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes"])
        impact = extract_section(text_output02, "IMPACT: Architectural Impact Assessment, that is Assessment of the likely functional impact and breadth of the changes",["ACTION_PLAN : A prioritised list of specific recommendations for improvement","SUMMARY : Comprehensive explanation of the contribution overall","END: ending by"])
        action_plan = extract_section(text_output02, "ACTION_PLAN : A prioritised list of specific recommendations for improvement",["SUMMARY : Comprehensive explanation of the contribution overall","END: ending by"])
        summary = extract_section(text_output02, "SUMMARY : Comprehensive explanation of the contribution overall",["END: ending by"])
        print(summary)

    except Exception as e:
        print(f">*< >*< >*< >*< >*< >*< >*< Parsing Error: {e}")
        return "Error", "Error", "Error", "Error", "Error", "Error"
      

    return purpose, positives, concerns, quality, impact, action_plan, summary

    
    #return text_output01, ap_text_output