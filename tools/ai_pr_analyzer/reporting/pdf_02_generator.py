import zlib
import base64
import markdown
import requests
from weasyprint import HTML



def generate_pdf_report(metrics, purpose, positives, concerns, quality, impact, action_plan, summary, pr_number, chart_b64, output_filename):
    """Compiles the HTML template and renders the final PDF."""

    # 1. Convert Markdown to raw HTML
    #raw_html01 = markdown.markdown(text_output01, extensions=['extra', 'codehilite'])
    #raw_html02 =markdown.markdown(ap_text_output, extensions=['extra', 'codehilite'])
    purpose_p = markdown.markdown(purpose, extensions=['extra', 'codehilite'])
    positives_p = markdown.markdown(positives, extensions=['extra', 'codehilite'])
    concerns_c = markdown.markdown(concerns, extensions=['extra', 'codehilite'])
    quality_q = markdown.markdown(quality, extensions=['extra', 'codehilite'])
    impact_i = markdown.markdown(impact, extensions=['extra', 'codehilite'])
    action_plan_ap = markdown.markdown(action_plan, extensions=['extra', 'codehilite'])
    summary_s = markdown.markdown(summary, extensions=['extra', 'codehilite'])


    
    
    total_size = metrics.get('additions', 0) + metrics.get('deletions', 0)
    files_changed = metrics.get('changed_files', len(metrics.get('file_names', [])))

    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <style>
            body {{ font-family: 'Helvetica Neue', Arial, sans-serif; color: #333; line-height: 1.6; margin: 20px; }}
            h1 {{ color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; }}
            h2 {{ color: #2980b9; margin-top: 30px; }}
            .dashboard {{ display: flex; justify-content: space-between; background: #ecf0f1; padding: 15px; border-radius: 8px; font-weight: bold; margin-bottom: 20px; }}
            .metric {{ text-align: center; width: 22%; }}
            .metric span {{ display: block; font-size: 24px; color: #e74c3c; }}
            .section-box {{ background: #f9f9f9; padding: 15px; border-left: 4px solid #3498db; margin-bottom: 15px; }}
            .chart-container {{ text-align: center; margin-top: 30px; page-break-before: always; }}
            img {{ max-width: 100%; height: auto; border: 1px solid #ddd; padding: 5px; }}
            svg {{ max-width: 100%; max-height: 400px; }}
        </style>
    </head>
    <body>
        <h1>BioDynaMo PR Diagnostics Report , PR {pr_number}</h1>
        <p><strong>PR Title:</strong> {metrics.get('title', 'N/A')}</p>
        
        <div class="dashboard">
            <div class="metric">Files Changed<span>{files_changed}</span></div>
            <div class="metric">Lines Added<span>+{metrics.get('additions', 0)}</span></div>
            <div class="metric">Lines Removed<span>-{metrics.get('deletions', 0)}</span></div>
            <div class="metric">Total PR Size<span>{total_size}</span></div>
        </div>

        <h2> Purpose & Overall Explanation</h2>
        <div class="section-box"><p>{purpose_p}</p></div>

         <div class="chart-container">
            <h2>2. Static Code Complexity Analysis</h2>
            <p>Files exceeding an average complexity of 10 should be reviewed for potential refactoring.</p>
            <img src="data:image/png;base64,{chart_b64}" alt="Complexity Chart"/>
        </div>

        <h2> Positives & Strengths</h2>
        <div class="section-box"><p>{positives_p}</p></div>

        <h2> Potential Concerns</h2>
        <div class="section-box"><p>{concerns_c}</p></div>

        <h2> Code Quality and Maintainability Considerations</h2>
        <div class="section-box"><p>{quality_q}</p></div>

         <h2> Architectural Impact</h2>
        <div class="section-box"><p>{impact_i}</p></div>

         <h2> Action plan</h2>
        <div class="section-box"><p>{action_plan_ap}</p></div>

         <h2> Summary</h2>
        <div class="section-box"><p>{summary_s}</p></div>

         

        
   
    </body>
    </html>
    """

    print(f"📄 Compiling Executive PDF Report...")
    HTML(string=html_content).write_pdf(output_filename)
    print(f"✅ Success! Report saved as: {output_filename}")