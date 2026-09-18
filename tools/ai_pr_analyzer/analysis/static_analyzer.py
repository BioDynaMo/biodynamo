import os
import base64
import lizard
import matplotlib.pyplot as plt


def generate_complexity_chart(file_data):
    #calculates cyclomatric complexity using lizard and output a Base64 PNG

    filenames, complexities = [], []
    for filename, code_string in file_data.items():
        #feeding raw c++ code into lizard, so it can scan and find the code complexity.
        analysis = lizard.analyze_file.analyze_source_code(filename,code_string)
        # we are complexity of the by taking avg code complexty as the sum of number of cyclomatic_complexty by number of functions. 
        avg_ccn = sum(f.cyclomatic_complexity for f in analysis.function_list) / len(analysis.function_list) if analysis.function_list else 0
        filenames.append(filename.split('/')[-1])
        complexities.append(avg_ccn)

    if not filenames:
        return None
    
    #plotting graph for file name vs code complexity

    plt.figure(figsize=(8,4))
    colors = ['#e74c34' if c > 10 else '#f39c12' if c > 5 else '#27ae60' for c in complexities]
    plt.bar(filenames, complexities, color = colors)
    plt.axhline( y= 10, color='r', linestyle ='--', label='High Risk (CCN > 10)')
    plt.title('Code Complexities Impact by File')
    plt.ylabel('Avg Clyclomatric Complexity')
    plt.xticks(rotation =45, ha= 'right')
    plt.tight_layout()

    chart_path = 'temp_complexity.png'
    plt.savefig(chart_path, dpi =150)
    plt.close()

    # reopening sved plot as read binary and encodes it into standard base64 string , so that it can be weasyprint(it can read base64 sting in html)
    #can save local memory 
    with open(chart_path, 'rb') as f:
        chart_b64 = base64.b64encode(f.read()).decode('utf-8')
    os.remove(chart_path)
    return chart_b64    
        

        





