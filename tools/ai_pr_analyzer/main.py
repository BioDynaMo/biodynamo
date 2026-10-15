import sys
from dotenv import load_dotenv

# Load environment variables before doing anything else
load_dotenv()

# Import our custom modules
from api.github_client import get_pr_data
from analysis.static_analyzer import generate_complexity_chart
from ai.llm_gemini import generate_ai_insights
from reporting.pdf_02_generator import generate_pdf_report

if __name__ == "__main__":
    OWNER = "BioDynaMo"
    REPO = "biodynamo"
    PR_NUMBER = 471

    if len(sys.argv) > 1:
        # sys.argv[1] is the number we passed from the YAML file
        PR_NUMBER = int(sys.argv[1]) 
        print(f"CI/CD Trigger Detected! Targeting PR #{PR_NUMBER}")
    
    print(f"Fetching data for {OWNER}/{REPO} PR #{PR_NUMBER}...")
    metrics, files_data = get_pr_data(OWNER, REPO, PR_NUMBER)
    
    print("*-----> Running static C++ complexity analysis...")
    chart_b64 = generate_complexity_chart(files_data)
    

    # Phase 3: AI Brain
    print("*---> Phase 3: Generating AI insights...")
    purpose, positives, concerns, quality, impact, action_plan, summary = generate_ai_insights(metrics, files_data)

    # Phase 4: Flowchart Rendering
    #print("🎨 Phase 4: Rendering architecture diagrams...")
    #mermaid_svg = render_mermaid_to_svg(mermaid_code)
    

    # Phase 5: PDF Assembly
    print("*----> Phase 5: Compiling final PDF report...")
    output_filename = f"PR_{PR_NUMBER}_Analysis.pdf"
    generate_pdf_report(metrics, purpose, positives, concerns, quality, impact, action_plan, summary, PR_NUMBER, chart_b64, output_filename)