import pandas as pd
from sklearn.svm import SVC
from sklearn.model_selection import KFold, cross_val_score
from sklearn.preprocessing import OneHotEncoder
from sklearn.pipeline import make_pipeline
import numpy as np

data = pd.read_csv('/Users/joshua/Downloads/weather.csv')

X = data[['temperature', 'humidity']]
onehot_encoder = OneHotEncoder(sparse_output=False)
y_encoded = onehot_encoder.fit_transform(data[['window_state']])
model = make_pipeline(SVC(kernel='linear', probability=True))
kf = KFold(n_splits=5, shuffle=True, random_state=42)

cross_val_scores_svm = cross_val_score(model, X, y_encoded.argmax(axis=1), cv=kf, scoring='accuracy')
model.fit(X, y_encoded.argmax(axis=1))

svm_clf = model.named_steps['svc']
coef_svm = svm_clf.coef_
intercept_svm = svm_clf.intercept_

print(f"Coefficients (Temperature and Humidity) for each class:\n{coef_svm}")
print(f"Intercepts for each class:\n{intercept_svm}")
print(f"Cross-validated accuracy scores for each fold: {cross_val_scores_svm}")
print(f"Mean accuracy: {np.mean(cross_val_scores_svm) * 100:.2f}%")
